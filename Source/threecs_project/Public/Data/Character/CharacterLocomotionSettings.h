// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ThreeCsPrimaryDataAsset.h"
#include "Character/Logic/CharacterState.h"
#include "CharacterLocomotionSettings.generated.h"

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API UCharacterLocomotionSettings : public UThreeCsPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Movement")
	FCharacterMovementSettings MovementSettings;

	UPROPERTY(EditAnywhere, Category = "Animation")
	FName RotationCurveName;
};
