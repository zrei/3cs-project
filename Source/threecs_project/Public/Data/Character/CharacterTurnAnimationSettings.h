// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ThreeCsPrimaryDataAsset.h"
#include "CharacterTurnAnimationSettings.generated.h"

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API UCharacterTurnAnimationSettings : public UThreeCsPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	TObjectPtr<UAnimSequenceBase> TurnLeftLessThan180Asset;

	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	TObjectPtr<UAnimSequenceBase> TurnRightLessThan180Asset;

	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	TObjectPtr<UAnimSequenceBase> TurnLeftMoreThan180Asset;

	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	TObjectPtr<UAnimSequenceBase> TurnRightMoreThan180Asset;

	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	FName LegsSlotName;
};
