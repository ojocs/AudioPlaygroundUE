// Fill out your copyright notice in the Description page of Project Settings.


#include "CubesSpawner.h"

// Sets default values
ACubesSpawner::ACubesSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CubesClockName = "QuartzCubesClock";
	// Init subsystem
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
	}
	
	// Set up clock to spawn cubes on metronome quantization events
	//CubesClock = GetWorld()->GetGameInstance()->GetSubsystem<UQuartzSubsystem>()->
	//	CreateNewClock(GetWorld(), CubesClockName, QuartzClockSettings);

	//QuartzMetronomeEvent.BindUFunction(GetWorld(), FName(TEXT("OnQuartzQuantizationEvents")));

	//CubesClock->SubscribeToAllQuantizationEvents(GetWorld(), QuartzMetronomeEvent, CubesClock);
}

// Called every frame
void ACubesSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//void ACubesSpawner::OnQuartzQuantizationEvents_Implementation(FName ClockName, EQuartzCommandQuantization QuantizationType, int32 NumBars, int32 Beat, float BeatFraction)
//{
//	switch (QuantizationType)
//	{
//	case EQuartzCommandQuantization::Beat:
//		SpawnAudioSource();
//		break;
//	};
//}

void ACubesSpawner::SpawnAudioSource_Implementation()
{
		
}

