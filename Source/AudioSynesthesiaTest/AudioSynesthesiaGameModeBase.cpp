#include "AudioSynesthesiaGameModeBase.h"
// Fill out your copyright notice in the Description page of Project Settings.

void AAudioSynesthesiaGameModeBase::CubeSpawnerDebugging(bool bShouldDebug)
{
	CubeSpawnerDebug(bShouldDebug);
}

void AAudioSynesthesiaGameModeBase::CubeSpawnerDebug(bool bShouldDebug)
{
	bCubeSpawnerDebug = bShouldDebug;
	OnCubeSpawnerDebugToggled.Broadcast(bCubeSpawnerDebug);
}
