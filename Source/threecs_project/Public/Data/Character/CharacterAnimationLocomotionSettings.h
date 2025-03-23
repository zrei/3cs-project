// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ThreeCsPrimaryDataAsset.h"
#include "CharacterAnimationLocomotionSettings.generated.h"

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API UCharacterAnimationLocomotionSettings : public UThreeCsPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Turn Settings")
	float MovingTurnBlendAlpha = 0.5;

	UPROPERTY(EditAnywhere, Category="Turn Settings")
	float StationaryTurnBlendAlpha = 1;

	UPROPERTY(EditAnywhere, Category="Locomotion")
	float JumpTime;
};
