// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/Logic/Base_MyCharacter.h"
#include "CharacterAnimationInstance.generated.h"

/**
 * UAnimInstance to function as the base class for the character animation BP.
 * Implements the logic to calculate the variables that will be used to drive the animation.
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
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

#pragma region Locomotion
private:
	void UpdateMovementState();

	void UpdateCharacterState();

	void UpdateStride();

	void OnCharacterMovementStateChanged(ECharacterMovementState prevState, ECharacterMovementState currState);

	void ResetJumpState();

	TObjectPtr<ABase_MyCharacter> CharacterRef;

	TObjectPtr<UCharacterMovementComponent> CharacterMovementRef;

	FCharacterMovementSettings CharacterMovementSettings;

	FCharacterState CurrCharacterState;

	FVector VelocityVector;

	FTimerHandle JumpTimerHandle;

protected:
	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	bool IsMoving;

	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	float MovementSpeed;

	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	float Stride;

	UPROPERTY(BlueprintReadOnly, Category="Locomotion")
	bool IsJumping;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Locomotion")
	float JumpTime;
#pragma endregion

#pragma region Turning
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Turn Settings")
	float MovingTurnBlendAlpha = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Turn Settings")
	float StationaryTurnBlendAlpha = 1;

	UPROPERTY(BlueprintReadOnly, Category="Turn Settings")
	float CurrTurnBlendAlpha;
#pragma endregion

#pragma region Look
protected:
	UPROPERTY(BlueprintReadOnly, Category="Look")
	float LookPitch;

	UPROPERTY(BlueprintReadOnly, Category="Look")
	float LookRoll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Look")
	float LookPitchInterpolationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Look")
	float LookRollInterpolationSpeed;

private:
	void UpdateLookState(float deltaTime);

	/*
	In world space
	*/
	float CurrLookPitch;

	/*
	In world space
	*/
	float CurrLookRoll;
#pragma endregion

#pragma region Feet IK Settings
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	FName LeftFootRootBoneName = FName{"root"};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	FName RightFootRootBoneName = FName{"root"};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	FName LeftIKFootBoneName = FName{ "ik_foot_l" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	FName RightIKFootBoneName = FName{ "ik_foot_r" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	FName LeftFootIKCurveName = FName{ "Enable_FootIK_L" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	FName RightFootIKCurveName = FName{ "Enable_FootIK_R" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	FName LeftFootLockCurve = FName{"FootLock_L"};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	FName RightFootLockCurve = FName{"FootLock_R"};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	float IKTraceDistanceAboveFoot = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	float IKTraceDistanceBelowFoot = 45;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feet IK Settings")
	float FootHeight = 13.5;
#pragma endregion

#pragma region Feet IK
protected:
	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	FVector FootLockLLocation;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	FRotator FootLockLRotation;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	float FootLockLAlpha;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	FVector FootLockRLocation;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	FRotator FootLockRRotation;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	float FootLockRAlpha;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	FVector FootOffsetLLocation;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	FRotator FootOffsetLRotation;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	FVector FootOffsetRLocation;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	FRotator FootOffsetRRotation;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	FVector PelvisOffset;

	UPROPERTY(BlueprintReadOnly, Category="Feet IK")
	float PelvisAlpha;

private:
	void UpdateFootIK(float deltaTime);

	void SetFootOffsets(float deltaTime, FName enableFootIKCurveName, FName IKFootBoneName, FName RootBoneName, FVector& currLocationTarget, FVector& currLocationOffset, FRotator& currRotationOffset);

	void SetPelvisIKOffset(float deltaTime, FVector footOffsetLTarget, FVector footOffsetRTarget);

	void SetFootLocking(float deltaTime, FName enableFootIKCurveName, FName footLockCurveName, FName IKFootBoneName, float& currentFootLockAlpha, FVector& currentFootLockLocation, FRotator& currentFootLockRotation);

	void SetFootLockOffsets(float deltaTime, FVector& localLocation, FRotator& localRotation);

	FVector FootOffsetLTarget;

	FVector FootOffsetRTarget;
#pragma endregion
};
