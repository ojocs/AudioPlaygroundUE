// Fill out your copyright notice in the Description page of Project Settings.


#include "CubesSpawner.h"

// Sets default values
ACubesSpawner::ACubesSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CubesClockName = "QuartzCubesClock";
	// Init subsystem

	PoolSize = 10.f;
}

// Called when the game starts or when spawned
void ACubesSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	// go through elements in pool set them up
	for (int32 i = 0; i < PoolSize; ++i)
	{
		UAudioComponent* audioSourceComponent = NewObject<UAudioComponent>(this);
		audioSourceComponent->RegisterComponent();
		audioSourceComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		audioSourcePool.Add(audioSourceComponent);

		if (soundObjectToSpawn)
		{
			AActor* spawnDuplicate = SpawnDuplication();
			spawnDuplicate->SetActorHiddenInGame(true);
			soundsObjects.Add(spawnDuplicate);
		}
	}
	//CubesClock->Init(GetWorld());

	// Set up clock to spawn cubes on metronome quantization events
	//CubesClock = GetWorld()->GetGameInstance()->GetSubsystem<UQuartzSubsystem>()->
	//	CreateNewClock(GetWorld(), CubesClockName, QuartzClockSettings);

	QuartzMetronomeEvent.BindUFunction(GetWorld(), FName(TEXT("OnQuartzQuantizationEvents")));
	// subscribe the metronome event above to a clock in BP

	//CubesClock->SubscribeToAllQuantizationEvents(GetWorld(), QuartzMetronomeEvent, CubesClock);
}

// Called every frame
void ACubesSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ACubesSpawner::RespositionCircleCenter(FVector CurrentCubePosition)
{
	// raycast a line down to see where the ground is, from our current position
	FCollisionQueryParams CircleTraceParams = FCollisionQueryParams(FName(TEXT("CircleTraceParams")), false, this);
	CircleTraceParams.bReturnPhysicalMaterial = false;

	//Re-initialize hit info
	FHitResult Circle_Hit(ForceInit);

	FVector EndTrace = CurrentCubePosition + (GetActorUpVector() * -1000.f);

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
		return Circle_Hit.ImpactPoint + GetActorUpVector() * SpawnCircleGroundBuffer;
	}

	return FVector();
}

UAudioComponent* ACubesSpawner::GetAvailableAudioSourceComponent()
{
	for (UAudioComponent* audioSourceComponent: audioSourcePool)
	{
		if (!audioSourceComponent->IsPlaying())
		{
			return audioSourceComponent;
		}
	}
	return nullptr;
}

AActor* ACubesSpawner::GetAvailableSoundObject()
{
	return nullptr;
}

void ACubesSpawner::OnQuartzQuantizationEvents_Implementation(FName ClockName, EQuartzCommandQuantization QuantizationType, int32 NumBars, int32 Beat, float BeatFraction)
{
	switch (QuantizationType)
	{
	case EQuartzCommandQuantization::Beat:
		SpawnAudioSource();
		break;
	};
}

// Called from base quartz quantization implementation
void ACubesSpawner::SpawnAudioSource_Implementation()
{
	// We've assigned something in BP
	if (soundObjectToSpawn)
	{
		FVector CurrentForwardSpawnPoint = GetActorLocation();
		for (int i = 0; i < PoolSize; ++i)
		{
			
			// 1st - forward location, push the spawn point up a bit
			CurrentForwardSpawnPoint = CurrentForwardSpawnPoint + (GetActorForwardVector() * HorizontalBufferSpace * i);

			// Adjust the vertical position
			CurrentForwardSpawnPoint = RespositionCircleCenter(CurrentForwardSpawnPoint);

			FVector NewSpawn = CurrentForwardSpawnPoint;

			// 2nd - get the point on our imaginary circle
			FColor color = FColor::Blue;
			//FMatrix matrix = FMatrix(FPlane(FVector(1.f, 0.f, 0.f)), FPlane(FVector(1.f, 0.f, 0.f)), FPlane(FVector(1.f, 0.f, 0.f)), FPlane());
			//DrawDebugCircle(GetWorld(), matrix, NewSpawn, SpawnCircleRadius, (int32)22, color, true);
			FVector2D circlePoint = FMath::RandPointInCircle(SpawnCircleRadius);
			NewSpawn += FVector(circlePoint.X, circlePoint.Y, 0.f);
			
			AActor* CurrentObject = soundsObjects[i];
			
			const bool IsInRange = FVector::Distance(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(), NewSpawn)
				<= SpawnRange;
			FColor pointColor = IsInRange ? FColor::Magenta : FColor::Yellow;
			DrawDebugPoint(GetWorld(), NewSpawn, 20.f, pointColor, true);

			// Make sure we are in range of the player
			//if (IsInRange)
			//{
			//	
			//	// Spawn it in
			//	CurrentObject->SetActorLocation(NewSpawn);
			//	CurrentObject->SetActorHiddenInGame(false);
			//}
			//else
			//{
			//	CurrentObject->SetActorHiddenInGame(true);
			//}
		}		
	}

	// If there are sounds TO play //the sounds we want to pick from
	//if (soundsCollection.Num() > 0.f)
	//{
	//	// Get one at random (could result in them playing all the same sound!)
	//	int32 Index = FMath::RandRange(0, soundsCollection.Num() - 1);
	//	USoundBase* selectedSound = soundsCollection[Index];

	//	if (selectedSound)
	//	{
	//		// Get a "speaker" with which to play the sound
	//		UAudioComponent* availableSoundSource = GetAvailableAudioSourceComponent();
	//		//If we could get a speaker (ie they were not all in use)
	//		if (availableSoundSource)
	//		{
	//			// set the sound, set random position for it, play it!
	//			availableSoundSource->SetSound(selectedSound);
	//			FVector NewLocation = GetActorLocation() + FMath::VRand() * FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius);
	//			availableSoundSource->SetWorldLocation(NewLocation);
	//			availableSoundSource->Play();
	//		}
	//	}
	//}
}

