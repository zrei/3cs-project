// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ThreeCsPrimaryDataAsset.h"
#include "CharacterAnimationSwingSettings.generated.h"

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API UCharacterAnimationSwingSettings : public UThreeCsPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Swing")
	float MaxLegRotation = 50;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Swing")
	float LegRotationInterpSpeed = 5;
};
