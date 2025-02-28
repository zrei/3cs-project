// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ThreeCsPrimaryDataAsset.h"
#include "RopeSettings.generated.h"

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API URopeSettings : public UThreeCsPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Base Setup")
	FString BonePrefix = FString{"Bone_"};

	UPROPERTY(EditAnywhere, Category="Base Setup", meta = (ClampMin = 1, ClampMax = 120, Tooltip = "Number of bones to make visible, starting from the top."))
	unsigned int VisibleBones = 120;

	UPROPERTY(EditAnywhere, Category="Base Setup", meta=(ClampMin = 0))
	float SwingCooldown = 10;

	UPROPERTY(EditAnywhere, Category="Physics Setup", meta=(ClampMin = 1, Tooltip="Bone, counted from the bottommost visible bone, that the rope should simulate physics from"))
	unsigned int BoneFromTheBottomToSimulatePhysics = 100;

	UPROPERTY(EditAnywhere, Category="Physics Setup", meta=(ClampMin = 1, Tooltip= "Bone, counted from the bottommost visible bone, that force should be applied on"))
	unsigned int BoneFromTheBottomToApplyForce = 50;
	
	UPROPERTY(EditAnywhere, Category="Physics Setup", meta=(ClampMin = 1, Tooltip="Number of bones, starting from the bottom, that are grabbable"))
	unsigned int NumberOfGrabbableBonesFromTheBottom = 33;

	UPROPERTY(EditAnywhere, Category="Physics Setup", meta=(ClampMin = 0))
	float RopeForce = 30000.0;

	UPROPERTY(EditAnywhere, Category="Visual Setup", meta=(ClampMin = 0))
	float CharacterOffset = 22;

	UPROPERTY(EditAnywhere, Category="Collision Setup", meta=(ClampMin = 0))
	float SphereColliderRadius = 25;
};
