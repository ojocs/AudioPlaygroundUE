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
class AAudioSynesthesiaGameModeBase;

USTRUCT(BlueprintType)
struct FSoundSpawnerElement
{
	GENERATED_BODY()

public:	
	FSoundSpawnerElement()
	{
		AActor* NullActor(nullptr);
		SoundObject = NullActor;
		TransformDestination = FTransform();
		CurrentSpawnLocationIndex = 0.f;
		bUsed = false;
	};

	FSoundSpawnerElement(AActor* InObject, FTransform InTransform, int32 InIndex, uint8 InUse)
		: SoundObject(InObject), TransformDestination(InTransform), CurrentSpawnLocationIndex(InIndex), bUsed(InUse){};

	// The actor that will be used
	UPROPERTY(BlueprintReadWrite)
	AActor* SoundObject;

	// The transform that our SoundObject will next lerp to
	UPROPERTY(BlueprintReadWrite)
	FTransform TransformDestination;

	// Points to the current spawn location in the SpawnLocations array to which our sound object revolves around
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentSpawnLocationIndex;

	// Is this object in use?
	UPROPERTY(BlueprintReadWrite)
	uint8 bUsed;

	// Set a new Z for TransformDestination's Scale
	void SetNewDestinationLocationZ(FVector InScale)
	{
		TransformDestination.SetScale3D(InScale);
	}
};

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

#pragma region Miscellaneous
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Miscellaneous")
	AAudioSynesthesiaGameModeBase* GameModeRef;
	
	/** reference to player character
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Miscellaneous")
	APawn* PlayerPawnRef;

	/**
	* Called when debug is toggled in the game mode
	*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Debug")
	void ToggleDebug(bool NewToggle);
#pragma endregion

#pragma region Resource Pools
public:
	// Array of our sound elements
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pools")
	TArray<FSoundSpawnerElement> soundElements;

	// Count of "speakers" 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pools", meta = (ClampMin = "1", UIMin = "1"))
	int32 PoolSize;
#pragma endregion

#pragma region SoundSpawnerElements Wrappers
public:
	/**
	* Wrapper function to set TransformDestination within a SoundSpawnerElement
	* @param InTransform A new Transform Destination
	*/
	UFUNCTION(BlueprintCallable, Category = "SoundSpawnerElement")
	void SoundElementSetTransformDestination(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, FTransform InTransform);

	/**
	* Wrapper function to set a SoundSpawnerElement's Scale on its TransformDestination variable
	* @param InScale The new scale we wish to set as our destination
	*/
	UFUNCTION(BlueprintCallable, Category = "SoundSpawnerElement")
	void SoundElementSetScale(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, FVector InScale);

	/**
	* Wrapper function to set the SoundObject within a SoundSpawnerElement
	* @param InSoundObject The new sound object
	*/
	UFUNCTION(BlueprintCallable, Category = "SoundSpawnerElement")
	void SoundElementSetSoundObject(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, UPARAM(ref) AActor* InSoundObject = nullptr);


	/**
	* Wrapper function to set bUsed within a SoundSpawnerElement
	* @param InBUsed A new is used value
	*/
	UFUNCTION(BlueprintCallable, Category = "SoundSpawnerElement")
	void SoundElementSetIsUsed(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, uint8 InBUsed);


		/**
	* Wrapper function to set index of spawn location within a SoundSpawnerElement
	* @param InCurrentSpawnLocationIndex A new index for the spawn location
	*/
	UFUNCTION(BlueprintCallable, Category = "SoundSpawnerElement")
	void SoundElementSetSpawnLocationIndex(UPARAM(ref) FSoundSpawnerElement& InSoundSpawnElements, int32 InCurrentSpawnLocationIndex);
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

	/** 
	* When do we change the transforms for our Sound Elements?
	* Acts as a pseudo difficulty level too
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	EQuartzCommandQuantization SpawnTimeQuantization;
	
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

	/**
	* Scaling multipliers for the spawned objects
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FVector ScaleMultiplier;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Spawning")
	int32 GetNearestSpawnIndex() { return NearestSpawnIndex;  }

#pragma region SpawnLocations
	/**
	* Locations at which we can spawn an object
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning|SpawnLocations")
	TArray<FVector> SpawnLocations;

	/**
	* How close we have to be to the last spawn location to increase spawn locations
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning|SpawnLocations")
	float DistanceToIncreaseSpawnLocations = 400.f;
	
	/**
	* When do we check for if we're near the last Spawn Location?
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning|SpawnLocations")
	EQuartzCommandQuantization CheckNearLastSpawnLocationTime;

	/**
	* How to increase spawn locations by when nearing the last spawn location
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning|SpawnLocations")
	int32 SpawnLocationsIncrement;

	/**
	* Finds a position from the ground at given height buffer
	* This is dynamic for terrain changes.
	* @param CurrentCubePosition The current position of the object
	* @param GroundBuffer The desired buffer distance between the object and the ground
	* @return The new buffered position
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Spawning|SpawnLocations")
	FVector FindBufferedPositionFromGround(FVector CurrentCubePosition, const float GroundBuffer);

	/**
	* Increase spawn locations array by the given amount from a start point
	* @param SizeIncrement The amount of spawn locations we wish to add onto our current array, starting at its end
	* @param StartingPosition The location where we begin to increment from
	*/
	UFUNCTION(BlueprintCallable, Category = "Spawning|SpawnLocations")
	void IncreaseSpawnLocations(int32 SizeIncrement, const FVector StartingPosition);


	/**
	* Are we in range of the last spawn location?
	* @return Are we in range?
	*/
	UFUNCTION(BlueprintCallable, Category = "Spawning|SpawnLocations")
	bool IsInRangeOfLastSpawnLocation();

private:
	/**
	* Index of current and nearest spawn location to the player
	*/
	int32 NearestSpawnIndex;
#pragma endregion

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

	// Function to get an available position within SpawnLocations
	uint32 CheckNextAvailablePosition();
#pragma endregion
};
