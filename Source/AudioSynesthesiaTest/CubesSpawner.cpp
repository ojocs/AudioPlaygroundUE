// Fill out your copyright notice in the Description page of Project Settings.


#include "CubesSpawner.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "AudioSynesthesiaGameModeBase.h"

// Only allow with editor, also change here to true/false for debugging
#define DEBUG (WITH_EDITOR && false)

// Sets default values
ACubesSpawner::ACubesSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CubesClockName = "QuartzCubesClock";
	// Init subsystem

	PoolSize = 10.f;
	NearestSpawnIndex = 0.f;
}

// Called when the game starts or when spawned
void ACubesSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerPawnRef = GetWorld()->GetFirstPlayerController()->GetPawn();
	GameModeRef = Cast<AAudioSynesthesiaGameModeBase>(GetWorld()->GetAuthGameMode());

	GameModeRef->OnCubeSpawnerDebugToggled.AddDynamic(this, &ACubesSpawner::ToggleDebug);
	//CubesClock->Init(GetWorld());

	// Set up clock to spawn cubes on metronome quantization events
	//CubesClock = GetWorld()->GetGameInstance()->GetSubsystem<UQuartzSubsystem>()->
	//	CreateNewClock(GetWorld(), CubesClockName, QuartzClockSettings);

	QuartzMetronomeEvent.BindUFunction(GetWorld(), FName(TEXT("OnQuartzQuantizationEvents")));
	// subscribe the metronome event above to a clock in BP

	//CubesClock->SubscribeToAllQuantizationEvents(GetWorld(), QuartzMetronomeEvent, CubesClock);



	InitSoundObjects();
}

// Called every frame
void ACubesSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACubesSpawner::InitSoundObjects_Implementation()
{
	// Set up spawn locations
	const FVector Origin = GetActorLocation();
	FVector CurrentForwardSpawnPoint = Origin;
	for (int i = 0; i < SpawnFrequencyBandsAmount; ++i)
	{
		// 1st - forward location, push the spawn point up a bit
		CurrentForwardSpawnPoint = FVector(Origin.X, Origin.Y + (HorizontalBufferSpace * i), Origin.Z);

		// Adjust the vertical position
		CurrentForwardSpawnPoint = RespositionCircleCenter(CurrentForwardSpawnPoint);
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

FVector ACubesSpawner::RespositionCircleCenter(FVector CurrentCubePosition)
{
	// raycast a line down to see where the ground is, from our current position
	FCollisionQueryParams CircleTraceParams = FCollisionQueryParams(FName(TEXT("CircleTraceParams")), false, this);
	CircleTraceParams.bReturnPhysicalMaterial = false;

	//Re-initialize hit info
	FHitResult Circle_Hit(ForceInit);

	FVector EndTrace = CurrentCubePosition + (GetActorUpVector() * -(1000.f + SpawnCircleRadius + SpawnCircleGroundBuffer));

	//call GetWorld() from within an actor extending class
	GetWorld()->LineTraceSingleByObjectType(
		Circle_Hit,		//result
		CurrentCubePosition,	//start
		EndTrace, //end
		FCollisionObjectQueryParams::AllStaticObjects, //collision channel
		CircleTraceParams
	);

	if (Circle_Hit.IsValidBlockingHit())
	{
		// place the center at: circle radius + buffer from ground
		return Circle_Hit.ImpactPoint + GetActorUpVector() * (SpawnCircleRadius + SpawnCircleGroundBuffer);
	}

	return FVector();
}

AActor* ACubesSpawner::GetAvailableSoundObject()
{
	return nullptr;
}

void ACubesSpawner::OnQuartzQuantizationEvents_Implementation(FName ClockName, EQuartzCommandQuantization QuantizationType, int32 NumBars, int32 Beat, float BeatFraction)
{
	if (QuantizationType == SpawnTimeQuantization)
	{
		SpawnAudioSource();
	}
}

// Called from base quartz quantization implementation
void ACubesSpawner::SpawnAudioSource_Implementation()
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
		// See which spawn point is closest, stop ourselves at the last PoolSize amount
		const float OldNearestSpawnIndex = NearestSpawnIndex;
		float DistanceToOldSpawnLocation = FVector::Dist2D(SpawnLocations[NearestSpawnIndex], PlayerLocation);
		for (int SpawnIndex = 0; SpawnIndex < SpawnFrequencyBandsAmount - PoolSize; ++SpawnIndex)
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
			SoundObjectRepositioning(CurrentPoolElement, CurrentSpawnIndex);

			// Update spawn index, careful to not go out beyond PoolSize limit
			if (CurrentSpawnIndex < SpawnFrequencyBandsAmount - PoolSize)
			{
				++CurrentSpawnIndex;
			}
		}
	}
}

void ACubesSpawner::SoundElementSetScale(UPARAM(ref)int32 ElementIndex, FVector InScale)
{
	FSoundSpawnerElement* element = &soundElements[ElementIndex]; 
	element->SetNewDestinationLocationZ(InScale);
}

void ACubesSpawner::SoundObjectRepositioning(int32 SoundObjectIndex, int32 SpawnLocationIndex)
{
	FVector NewSpawnOnCircle = SpawnLocations[SpawnLocationIndex];

	// 2nd - get a point on our imaginary circle
	const float RandomAngle = FMath::FRandRange(0.f, 360.f);
	// random angle on a circle's circumference: Cosine = x , sine = y ; therefore -- r * cosine(angle) = X && r * sine(angle) = Y --> give random point
	FVector2D circlePoint = FVector2D(SpawnCircleRadius * (FMath::Cos(RandomAngle)), SpawnCircleRadius * (FMath::Sin(RandomAngle)));
	NewSpawnOnCircle += FVector(circlePoint.X, 0.f, circlePoint.Y);

	const bool IsInRange = FVector::Distance(PlayerPawnRef->GetActorLocation(), SpawnLocations[SpawnLocationIndex])
		<= SpawnRange;
	/* Debugging */
	if (GameModeRef && GameModeRef->GetCubeSpawnerDebug())
	{
		FColor color = FColor::Blue;
		DrawDebugCircle(GetWorld(), SpawnLocations[SpawnLocationIndex], SpawnCircleRadius, (int32)22, color, true, -1.f, (uint8)0U, 3.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), true);
		FColor pointColor = IsInRange ? FColor::Magenta : FColor::Yellow;
		DrawDebugPoint(GetWorld(), NewSpawnOnCircle, 20.f, pointColor, true);
	}

	FSoundSpawnerElement* SoundElement = &soundElements[SoundObjectIndex];
	/*if (IsInRange)
	{*/
		FRotator NewRotation = FRotationMatrix::MakeFromYZ(FVector(0.f, 1.f, 0.f), (NewSpawnOnCircle - SpawnLocations[SpawnLocationIndex])).Rotator();

		SoundElement->TransformDestination.SetRotation(FQuat(NewRotation));
		SoundElement->TransformDestination.SetLocation(NewSpawnOnCircle);
		SoundElement->CurrentSpawnLocationIndex = SpawnLocationIndex;
		// Disable the last object
		/*AActor* LastPoolObject = soundsObjects[PoolSize - 1];
		LastPoolObject->SetActorHiddenInGame(true);
		LastPoolObject->SetActorEnableCollision(false);*/
	//}

	SoundElement->SoundObject->SetActorHiddenInGame(!IsInRange);
	SoundElement->SoundObject->SetActorEnableCollision(IsInRange);
}
