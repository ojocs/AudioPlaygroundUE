// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Quartz/QuartzSubsystem.h"
#include "Quartz/AudioMixerClockHandle.h"
#include "Sound/QuartzQuantizationUtilities.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "CubesSpawner.generated.h"


UCLASS()
class AUDIOSYNESTHESIATEST_API ACubesSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACubesSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#pragma region Resource Pools
	// Array of sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<USoundBase*> soundsCollection;

	// Array of objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<AActor*> soundsObjects;

	// Count of "speakers" 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "1", UIMin = "1"))
	int32 PoolSize;
#pragma endregion

#pragma region Spawn Logic
public:
	// Audio Source Spawning Logic
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SpawnAudioSource();
	
	// Duplicate our soundObjectToSpawn. For use in BeginPlay
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	AActor* SpawnDuplication();

	// The object to which our sounds will be attached to. Will be duplicated up to PoolSize
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	AActor* soundObjectToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float CustomSpawnTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float MinSpawnRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float MaxSpawnRadius = 500.0f;

	/** Horizontal buffer space between each spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float HorizontalBufferSpace = 50.f;

	/** The radius of our invisible circle on which we can spawn onto */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SpawnCircleRadius = 500.f;

	/** The buffer of our invisible circle from the ground */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SpawnCircleGroundBuffer = 200.f;

	/** The range in which we reveal the pooled object to the player. Otherwise, keep actor hidden away */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SpawnRange = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float ZAxisScaling = 2.f;

private:
	/** 
	* Function that repositions the center point for the imaginary circle.
	* This is dynamic for terrain changes.
	*/
	FVector RespositionCircleCenter(FVector CurrentCubePosition);

#pragma endregion

#pragma region Clock
public:
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuartzClock")
	UQuartzClockHandle* CubesClock;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuartzClock")
	FName CubesClockName;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuartzClock")
	//FQuartzClockSettings QuartzClockSettings;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnQuartzQuantizationEvents(FName ClockName, EQuartzCommandQuantization QuantizationType, int32 NumBars, int32 Beat, float BeatFraction);
private:
	FOnQuartzMetronomeEventBP QuartzMetronomeEvent;
//
//	// Clock
//	FTimerHandle TimerHandle;

#pragma endregion

private:
#pragma region Resource Pools
	// The "speakers" themselves
	TArray<UAudioComponent*> audioSourcePool;

	// Function to get an available "speaker" from the source pool
	UAudioComponent* GetAvailableAudioSourceComponent();

	// Function to get available sound object in the pool
	AActor* GetAvailableSoundObject();
#pragma endregion
};
