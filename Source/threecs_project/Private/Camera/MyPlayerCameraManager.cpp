// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/MyPlayerCameraManager.h"
#include "Rope.h"

void AMyPlayerCameraManager::BeginPlay()
{
	Super::BeginPlay();

	ARope::RopeAttachEvent.AddUObject(this, &AMyPlayerCameraManager::RopeAttach);
}

void AMyPlayerCameraManager::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);

	ARope::RopeAttachEvent.RemoveAll(this);
}

void AMyPlayerCameraManager::RopeAttach(ARope* const ropeActor)
{
	SetViewTarget(ropeActor);
}