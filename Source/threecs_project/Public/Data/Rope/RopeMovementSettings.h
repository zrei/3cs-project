// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ThreeCsPrimaryDataAsset.h"
#include "RopeMovementSettings.generated.h"

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API URopeMovementSettings : public UThreeCsPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float CameraHorizontalSpeed = 2;

	UPROPERTY(EditAnywhere)
	float CharacterHorizontalShimmySpeed = 1;
};
