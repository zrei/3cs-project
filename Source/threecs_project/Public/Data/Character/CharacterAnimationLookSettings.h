// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ThreeCsPrimaryDataAsset.h"
#include "CharacterAnimationLookSettings.generated.h"

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API UCharacterAnimationLookSettings : public UThreeCsPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Look")
	float LookPitchInterpolationSpeed;

	UPROPERTY(EditAnywhere, Category="Look")
	float LookYawInterpolationSpeed;

	UPROPERTY(EditAnywhere, Category="Look")
	float RotatingLookYawInterpolationSpeed;
};
