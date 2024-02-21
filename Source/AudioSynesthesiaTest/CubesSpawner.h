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

class UEditorActorSubsystem;

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
	
	// Array of objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<AActor*> soundsObjects;
	
	// Array of objects' destinations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<FVector> soundsObjectsDestinations;
	
	// Array of objects' new rotations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<FRotator> soundsObjectsRotations;

	// Count of "speakers" 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "1", UIMin = "1"))
	int32 PoolSize;
#pragma endregion

#pragma region Spawn Logic
public:
	// Audio Source Spawning Logic
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SpawnAudioSource();

	/**
	* Reposition the indexed sound object to the correct spot on our imaginary circle
	* @param SoundObjectIndex The index of the sound object
	* @param SpawnLocationIndex The index of the current spawn location
	*/
	UFUNCTION(BlueprintCallable)
	void SoundObjectRepositioning(int32 SoundObjectIndex, int32 SpawnLocationIndex);

	// Sound Objects Spawning Logic
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void InitSoundObjects();
	
	// The type of object we will spawn using our pools
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AActor> SpawnerObjectClass = AActor::StaticClass();

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

	/** Number of total frequency bands we will be using overall. Is usually a greater number than PoolSize. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 SpawnFrequencyBandsAmount = 48.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float ZAxisScaling = 2.f;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Spawning")
	int32 GetNearestSpawnIndex() { return NearestSpawnIndex;  }


	/**
	* Locations at which we can spawn an object
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<FVector> SpawnLocations;

private:
	/** 
	* Function that repositions the center point for the imaginary circle.
	* This is dynamic for terrain changes.
	*/
	FVector RespositionCircleCenter(FVector CurrentCubePosition);

	/**
	* Index of current and nearest spawn location to the player
	*/
	int32 NearestSpawnIndex;

	/** reference to player character
	*/
	APawn* PlayerPawnRef;

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

	// Function to get available sound object in the pool
	AActor* GetAvailableSoundObject();
#pragma endregion
};
