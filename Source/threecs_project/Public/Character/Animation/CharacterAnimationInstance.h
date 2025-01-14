// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/Logic/Base_MyCharacter.h"
#include "CharacterAnimationInstance.generated.h"

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API UCharacterAnimationInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	void AnimationUpdate(float deltaTime);

	UFUNCTION(BlueprintCallable)
	void InitializeCharacterReferences();

private:
	void UpdateMovementState();
	
	void UpdateCharacterState();

	void UpdateStride();

	void UpdateInPlaceRotation();

	void SetRotateValues(bool rotateL, bool rotateR);

	TObjectPtr<ABase_MyCharacter> CharacterRef;

	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	TObjectPtr<UCharacterMovementComponent> CharacterMovementRef;

	FCharacterMovementSettings CharacterMovementSettings;

	FCharacterState CurrCharacterState;

protected:
	UPROPERTY(BlueprintReadOnly)
	bool IsMoving;

	UPROPERTY(BlueprintReadOnly)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly)
	float MovementSpeed;

	UPROPERTY(BlueprintReadOnly)
	float Stride;

	UPROPERTY(BlueprintReadOnly)
	bool RotateLeft;

	UPROPERTY(BlueprintReadOnly)
	bool RotateRight;

#pragma region Foot IK
protected:
	UPROPERTY(EditAnywhere)
	FName LeftFootRootBoneName = FName{"root"};

	UPROPERTY(EditAnywhere)
	FName RightFootRootBoneName = FName{"root"};

	UPROPERTY(EditAnywhere)
	FName LeftIKFootBoneName = FName{ "ik_foot_l" };

	UPROPERTY(EditAnywhere)
	FName RightIKFootBoneName = FName{ "ik_foot_r" };

	UPROPERTY(EditAnywhere)
	FName LeftFootIKCurveName = FName{ "Enable_FootIK_L" };;

	UPROPERTY(EditAnywhere)
	FName RightFootIKCurveName = FName{ "Enable_FootIK_R" };;

	UPROPERTY(EditAnywhere)
	float IKTraceDistanceAboveFoot = 50;

	UPROPERTY(EditAnywhere)
	float IKTraceDistanceBelowFoot = 45;

	UPROPERTY(EditAnywhere)
	float FootHeight = 13.5;

	UPROPERTY(BlueprintReadOnly)
	FVector FootLockLLocation;

	UPROPERTY(BlueprintReadOnly)
	FRotator FootLockLRotation;

	UPROPERTY(BlueprintReadOnly)
	float FootLockLAlpha;

	UPROPERTY(BlueprintReadOnly)
	FVector FootLockRLocation;

	UPROPERTY(BlueprintReadOnly)
	FRotator FootLockRRotation;

	UPROPERTY(BlueprintReadOnly)
	float FootLockRAlpha;

	UPROPERTY(BlueprintReadOnly)
	FVector FootOffsetLLocation;

	UPROPERTY(BlueprintReadOnly)
	FRotator FootOffsetLRotation;

	UPROPERTY(BlueprintReadOnly)
	FVector FootOffsetLTarget;

	UPROPERTY(BlueprintReadOnly)
	FVector FootOffsetRLocation;

	UPROPERTY(BlueprintReadOnly)
	FRotator FootOffsetRRotation;

	UPROPERTY(BlueprintReadOnly)
	FVector FootOffsetRTarget;

	UPROPERTY(BlueprintReadOnly)
	FVector PelvisOffset;

	UPROPERTY(BlueprintReadOnly)
	float PelvisAlpha;

private:
	void UpdateFootIK(float deltaTime);

	void SetFootOffsets(float deltaTime, FName enableFootIKCurveName, FName IKFootBoneName, FName RootBoneName, FVector& currLocationTarget, FVector& currLocationOffset, FRotator& currRotationOffset);

	void SetPelvisIKOffset(float deltaTime);

	void SetFootLocking(float deltaTime);

	void SetFootLockOffsets(float deltaTime);
#pragma endregion
};
