// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ThreeCsPrimaryDataAsset.h"
#include "CharacterAnimationFeetIKSettings.generated.h"

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API UCharacterAnimationFeetIKSettings : public UThreeCsPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Feet IK Settings")
	FName LeftFootRootBoneName = FName{"root"};

	UPROPERTY(EditAnywhere, Category="Feet IK Settings")
	FName RightFootRootBoneName = FName{"root"};

	UPROPERTY(EditAnywhere, Category="Feet IK Settings")
	FName LeftIKFootBoneName = FName{ "ik_foot_l" };

	UPROPERTY(EditAnywhere, Category="Feet IK Settings")
	FName RightIKFootBoneName = FName{ "ik_foot_r" };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Feet IK Settings")
	FName LeftFootIKCurveName = FName{ "Enable_FootIK_L" };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Feet IK Settings")
	FName RightFootIKCurveName = FName{ "Enable_FootIK_R" };

	UPROPERTY(EditAnywhere, Category="Feet IK Settings")
	FName LeftFootLockCurve = FName{"FootLock_L"};

	UPROPERTY(EditAnywhere, Category="Feet IK Settings")
	FName RightFootLockCurve = FName{"FootLock_R"};

	UPROPERTY(EditAnywhere, Category="Feet IK Settings")
	float IKTraceDistanceAboveFoot = 50;

	UPROPERTY(EditAnywhere, Category="Feet IK Settings")
	float IKTraceDistanceBelowFoot = 45;

	UPROPERTY(EditAnywhere, Category="Feet IK Settings")
	float FootHeight = 13.5;
};
