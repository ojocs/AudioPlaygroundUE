// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AudioSynesthesiaGameModeBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCubeSpawnerDebuggingToggle, bool, NewToggle);

/**
 * 
 */
UCLASS()
class AUDIOSYNESTHESIATEST_API AAudioSynesthesiaGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
#pragma region Debugging
public:
	/** Fires when the debug is toggled */
	UPROPERTY(BlueprintAssignable)
	FCubeSpawnerDebuggingToggle OnCubeSpawnerDebugToggled;

	/** 
	* Should we debug the cube spawner?
	* @param bShouldDebug Toggle debugging
	*/
	UFUNCTION(Exec, Category= "Commands")
	void CubeSpawnerDebugging(bool bShouldDebug);

	/**
	* Should we debug the cube spawner?
	* @param bShouldDebug Toggle debugging
	*/
	UFUNCTION(BlueprintCallable, Category = "Debugging")
	void CubeSpawnerDebug(bool bShouldDebug);

	/** Can we debug? */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Debugging")
	bool GetCubeSpawnerDebug() { return bCubeSpawnerDebug; }

private:
	// The toggle for debugging the cube spawner elements
	bool bCubeSpawnerDebug;
#pragma endregion
};
