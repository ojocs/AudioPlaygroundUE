// Fill out your copyright notice in the Description page of Project Settings.


#include "CubesSpawner.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "AudioSynesthesiaGameModeBase.h"

// Only allow with editor, also change here to true/false for debugging
#define DEBUG (WITH_EDITOR && false)

#pragma region Basic and Overridden
// Sets default values
ACubesSpawner::ACubesSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CubesClockName = "QuartzCubesClock";
	// Init subsystem

	PoolSize = 10.f;
	NearestSpawnIndex = 0.f;

	CheckNearLastSpawnLocationTime = EQuartzCommandQuantization::QuarterNote;
	SpawnLocationsIncrement = 20.f;
}

// Called when the game starts or when spawned
void ACubesSpawner::BeginPlay()
{
	// Init refs
	PlayerPawnRef = GetWorld()->GetFirstPlayerController()->GetPawn();
	GameModeRef = Cast<AAudioSynesthesiaGameModeBase>(GetWorld()->GetAuthGameMode());

	// Set up delegate callbacks
	GameModeRef->OnCubeSpawnerDebugToggled.AddDynamic(this, &ACubesSpawner::ToggleDebug);
	OnCubeSpawnerSpawnLocationsIncreased.AddDynamic(this, &ACubesSpawner::SpawnLocationIncreased);
	
	//CubesClock->Init(GetWorld());

	// Set up clock to spawn cubes on metronome quantization events
	//CubesClock = GetWorld()->GetGameInstance()->GetSubsystem<UQuartzSubsystem>()->
	//	CreateNewClock(GetWorld(), CubesClockName, QuartzClockSettings);

	QuartzMetronomeEvent.BindUFunction(GetWorld(), FName(TEXT("OnQuartzQuantizationEvents")));
	// subscribe the metronome event above to a clock in BP

	//CubesClock->SubscribeToAllQuantizationEvents(GetWorld(), QuartzMetronomeEvent, CubesClock);
	
	// Set up spawn locations
	IncreaseSpawnLocations(SpawnFrequencyBandsAmount, GetActorLocation());

	InitSoundObjects();

	Super::BeginPlay();
}

// Called every frame
void ACubesSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#pragma endregion

#pragma region Quartz

void ACubesSpawner::OnQuartzQuantizationEvents_Implementation(FName ClockName, EQuartzCommandQuantization QuantizationType, int32 NumBars, int32 Beat, float BeatFraction)
{
	if (QuantizationType == SpawnTimeQuantization)
	{
		SpawnSoundObjects();
	}

	if (QuantizationType == CheckNearLastSpawnLocationTime)
	{
		// Check if player is in range of last spawn location
		if (IsInRangeOfLastSpawnLocation())
		{
			// Increase spawn locations
			IncreaseSpawnLocations(SpawnLocationsIncrement, SpawnLocations[SpawnLocations.Num() - 1]);
		}
	}
}

#pragma endregion

#pragma region Sound Objects/Elements

void ACubesSpawner::InitSoundObjects_Implementation()
{
	// Set up Pool
	for (int32 i = 0; i < PoolSize; ++i)
	{
		if (IsValid(SpawnerObjectClass))
		{
			UWorld* CurrentWorld = GetWorld();
			if (IsValid(CurrentWorld))
			{
				FTransform SpawnTransform = FTransform(SpawnLocations[i]);

				AActor* spawnDuplicate = CurrentWorld->SpawnActor<AActor>(SpawnerObjectClass, SpawnTransform);
				spawnDuplicate->SetActorHiddenInGame(true);
				FSoundSpawnerElement NewSoundElement(spawnDuplicate, spawnDuplicate->GetActorTransform(), i, true);
				soundElements.Add(NewSoundElement);

				// Place correctly
				SoundObjectRepositioning(i, i);

				// You should validate the actor pointer before accessing it in case the Spawn failed.
				if (IsValid(spawnDuplicate))
				{
					UE_LOG(LogTemp, Log, TEXT("Spawned successfully! New Actor: %s"), *spawnDuplicate->GetName());
				}
			}
		}
	}
}

FVector ACubesSpawner::FindBufferedPositionFromGround(FVector CurrentCubePosition, const float GroundBuffer)
{
	// raycast a line down to see where the ground is, from our current position
	FCollisionQueryParams ObjectTraceParams = FCollisionQueryParams(FName(TEXT("CircleTraceParams")), false, this);
	ObjectTraceParams.bReturnPhysicalMaterial = false;

	//Re-initialize hit info
	FHitResult ObjectHit(ForceInit);
	// Do a really long trace, just to see where we hit
	FVector EndTrace = CurrentCubePosition + (GetActorUpVector() * -(1000.f + GroundBuffer));

	//call GetWorld() from within an actor extending class
	GetWorld()->LineTraceSingleByObjectType(
		ObjectHit,
		CurrentCubePosition,	//start
		EndTrace,
		FCollisionObjectQueryParams::AllStaticObjects, //collision channel
		ObjectTraceParams
	);

	if (ObjectHit.IsValidBlockingHit())
	{
		// object buffered at the given distance!
		return ObjectHit.ImpactPoint + GetActorUpVector() * GroundBuffer;
	}
	// Nothing, just keep it at the current spot
	return CurrentCubePosition;
}

// Called from base quartz quantization implementation
void ACubesSpawner::SpawnSoundObjects_Implementation()
{
	// We've assigned something in BP
	if (IsValid(SpawnerObjectClass))
	{
		const bool bInvalidPlayerRef = !IsValid(PlayerPawnRef);
		FVector PlayerLocation = bInvalidPlayerRef ? GetActorLocation() : PlayerPawnRef->GetActorLocation();;
		if (bInvalidPlayerRef)
		{
			PlayerPawnRef = GetWorld()->GetFirstPlayerController()->GetPawn();
			if (IsValid(PlayerPawnRef))
			{
				PlayerLocation = PlayerPawnRef->GetActorLocation();
			}
		}
		// See which spawn point is closest
		const int32 SpawnLocationsLength = SpawnLocations.Num();

		const float OldNearestSpawnIndex = NearestSpawnIndex;
		float DistanceToOldSpawnLocation = FVector::Dist2D(SpawnLocations[NearestSpawnIndex], PlayerLocation);
		for (int SpawnIndex = 0; SpawnIndex < SpawnLocationsLength; ++SpawnIndex)
		{
			const float DistanceToLocation = FVector::Dist2D(SpawnLocations[SpawnIndex], PlayerPawnRef->GetActorLocation());
			if (DistanceToLocation < DistanceToOldSpawnLocation)
			{
				NearestSpawnIndex = SpawnIndex;
				DistanceToOldSpawnLocation = DistanceToLocation;
			}
		}
		
		// @TODO: If value is negative we need to go backwards
		const int NumIndexesToChange = FMath::Abs(NearestSpawnIndex - OldNearestSpawnIndex);
		
		int CurrentSpawnIndex = NearestSpawnIndex;
		for (int CurrentPoolElement = 0; CurrentPoolElement < PoolSize; ++CurrentPoolElement)
		{
			// Update spawn index, careful to not go out beyond PoolSize limit
			if (CurrentSpawnIndex < SpawnLocationsLength)
			{
				SoundObjectRepositioning(CurrentPoolElement, CurrentSpawnIndex);
				++CurrentSpawnIndex;
			}
		}
	}
}

void ACubesSpawner::SoundObjectRepositioning(int32 SoundObjectIndex, int32 SpawnLocationIndex)
{
	FVector NewSpawnOnCircle = SpawnLocations[SpawnLocationIndex];

	// 2nd - get a point on our imaginary circle
	const float RandomAngle = FMath::FRandRange(0.f, 360.f);
	// random angle on a circle's circumference: Cosine = x , sine = y ; therefore -- r * cosine(angle) = X && r * sine(angle) = Y --> give random point
	FVector2D circlePoint = FVector2D(SpawnCircleRadius * (FMath::Cos(RandomAngle)), SpawnCircleRadius * (FMath::Sin(RandomAngle)));
	NewSpawnOnCircle += FVector(circlePoint.X, 0.f, circlePoint.Y);

	const bool IsInVisibleRange = FVector::Distance(PlayerPawnRef->GetActorLocation(), SpawnLocations[SpawnLocationIndex])
		<= SpawnRange;

	/*const bool ShouldTeleport = FVector::Distance(PlayerPawnRef->GetActorLocation(), SpawnLocations[SpawnLocationIndex])
		> SpawnRange * 0.5f;*/
	/* Debugging */
	if (GameModeRef && GameModeRef->GetCubeSpawnerDebug())
	{
		FColor color = FColor::Blue;
		DrawDebugCircle(GetWorld(), SpawnLocations[SpawnLocationIndex], SpawnCircleRadius, (int32)22, color, true, -1.f, (uint8)0U, 3.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), true);
		FColor pointColor = IsInVisibleRange ? FColor::Magenta : FColor::Yellow;
		DrawDebugPoint(GetWorld(), NewSpawnOnCircle, 20.f, pointColor, true);
	}

	FSoundSpawnerElement* SoundElement = &soundElements[SoundObjectIndex];

	FRotator NewRotation = FRotationMatrix::MakeFromYZ(FVector(0.f, 1.f, 0.f), (NewSpawnOnCircle - SpawnLocations[SpawnLocationIndex])).Rotator();
	SoundElement->TransformDestination.SetRotation(FQuat(NewRotation));
	/*if (ShouldTeleport)
	{*/
		
		SoundElement->TransformDestination.SetLocation(NewSpawnOnCircle);
		SoundElement->CurrentSpawnLocationIndex = SpawnLocationIndex;
		SoundElement->bUsed = IsInVisibleRange;
		// Disable the last object
		/*AActor* LastPoolObject = soundsObjects[PoolSize - 1];
		LastPoolObject->SetActorHiddenInGame(true);
		LastPoolObject->SetActorEnableCollision(false);*/
	//}

	SoundElement->SoundObject->SetActorHiddenInGame(!IsInVisibleRange);
	SoundElement->SoundObject->SetActorEnableCollision(IsInVisibleRange);
}

#pragma endregion

#pragma region Spawn Locations

bool ACubesSpawner::IsInRangeOfLastSpawnLocation()
{
	const FVector LastSpawnLocation = SpawnLocations[SpawnLocations.Num() - 1];
	const FVector PlayerLocation = PlayerPawnRef->GetActorLocation();
	return FVector::Distance(LastSpawnLocation, PlayerLocation) <= DistanceToIncreaseSpawnLocations;
}

void ACubesSpawner::IncreaseSpawnLocations(int32 SizeIncrement, const FVector StartingPosition)
{
	FVector CurrentForwardSpawnPoint = StartingPosition;
	for (int i = 0; i < SizeIncrement; ++i)
	{
		// 1st - forward location, push the spawn point up a bit
		CurrentForwardSpawnPoint = FVector(StartingPosition.X, StartingPosition.Y + (HorizontalBufferSpace * i), StartingPosition.Z);

		// Adjust the vertical position
		CurrentForwardSpawnPoint = FindBufferedPositionFromGround(CurrentForwardSpawnPoint, SpawnCircleRadius + SpawnCircleGroundBuffer);
		// Save spawn location
		SpawnLocations.Add(CurrentForwardSpawnPoint);

		/* Debugging */
		if (GameModeRef && GameModeRef->GetCubeSpawnerDebug())
		{
			FColor color = FColor::Blue;
			DrawDebugCircle(GetWorld(), CurrentForwardSpawnPoint, SpawnCircleRadius, (int32)22, color, true, -1.f, (uint8)0U, 3.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), true);
			DrawDebugPoint(GetWorld(), CurrentForwardSpawnPoint, 20.f, FColor::Magenta, true);

		}
	}

	OnCubeSpawnerSpawnLocationsIncreased.Broadcast(SpawnLocations);
}

void ACubesSpawner::SpawnLocationIncreased_Implementation(TArray<FVector>& NewSpawnLocations)
{
	// Implement in BP or here
}

#pragma endregion

#pragma region Sound Spawner Elements Wrappers
void ACubesSpawner::SoundElementSetTransformDestination(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, FTransform InTransform)
{
	InSoundSpawnElements.TransformDestination = InTransform;
}

void ACubesSpawner::SoundElementSetScale(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, FVector InScale)
{
	InSoundSpawnElements.SetNewDestinationLocationZ(InScale);
}

void ACubesSpawner::SoundElementSetSoundObject(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, UPARAM(ref)AActor* InSoundObject)
{
	if (InSoundObject)
	{
		InSoundSpawnElements.SoundObject = InSoundObject;
	}
}

void ACubesSpawner::SoundElementSetIsUsed(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, uint8 InBUsed)
{
	InSoundSpawnElements.bUsed = InBUsed;
}

void ACubesSpawner::SoundElementSetSpawnLocationIndex(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, int32 InCurrentSpawnLocationIndex)
{
	InSoundSpawnElements.CurrentSpawnLocationIndex = InCurrentSpawnLocationIndex;
}
#pragma endregion
