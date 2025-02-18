// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/MyPlayerCameraManager.h"
#include "Rope.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AMyPlayerCameraManager::BeginPlay()
{
	Super::BeginPlay();

	ARope::RopeAttachEvent.AddUObject(this, &AMyPlayerCameraManager::RopeAttach);
	ARope::RopeDetachEvent.AddUObject(this, &AMyPlayerCameraManager::RopeDetach);
}

void AMyPlayerCameraManager::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);

	ARope::RopeAttachEvent.RemoveAll(this);
	ARope::RopeDetachEvent.RemoveAll(this);
}

void AMyPlayerCameraManager::RopeAttach(ARope* const ropeActor)
{
	SetViewTarget(ropeActor);
}

void AMyPlayerCameraManager::RopeDetach()
{
	SetViewTarget(Cast<AActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)));
}