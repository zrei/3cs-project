// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class ARope;

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "MyPlayerCameraManager.generated.h"

/**
 * Create this first, to be utilised later
 */
UCLASS()
class THREECS_PROJECT_API AMyPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void RopeAttach(ARope* const);
};
