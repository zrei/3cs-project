// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UCharacterAnimationLocomotionSettings;
class UCharacterAnimationFeetIKSettings;
class UCharacterAnimationLookSettings;
class UCharacterAnimationSwingSettings;

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

	bool IsRunningInGame = false;

	
#pragma region Locomotion Settings
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Locomotion Settings")
	TObjectPtr<UCharacterAnimationLocomotionSettings> LocomotionSettings;
#pragma endregion

#pragma region Locomotion
private:
	void UpdateMovementState();

	void UpdateCharacterState();

	void UpdateStride();

	void OnCharacterMovementStateChanged(ECharacterMovementState prevState, ECharacterMovementState currState);

	void ResetJumpState();

	TObjectPtr<ABase_MyCharacter> CharacterRef;

	TObjectPtr<UCharacterMovementComponent> CharacterMovementRef;

	FVector VelocityVector;

	FTimerHandle JumpTimerHandle;

	ECharacterMovementState CurrMovementState = ECharacterMovementState::IDLE;

protected:
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Locomotion")
	bool IsMoving = false;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Locomotion")
	bool IsFalling = false;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Locomotion")
	float MovementSpeed = 0;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Locomotion")
	float Stride = 0;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Locomotion")
	bool IsJumping = false;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Locomotion")
	bool IsSwinging = false;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Locomotion")
	bool IsInAir = false;
#pragma endregion

#pragma region Turning
protected:
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Turn")
	float CurrTurnBlendAlpha;
#pragma endregion

#pragma region Look Settings
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Look Settings")
	TObjectPtr<UCharacterAnimationLookSettings> LookSettings;
#pragma endregion

#pragma region Look
protected:
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Look")
	float LookPitch = 0;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Look")
	float LookYaw = 0.5;

private:
	void UpdateLookState(float deltaTime);
#pragma endregion

#pragma region Feet IK Settings
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|Feet IK Settings")
	TObjectPtr<UCharacterAnimationFeetIKSettings> FeetIKSettings;
#pragma endregion

#pragma region Feet IK
protected:
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	FVector FootLockLLocation;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	FRotator FootLockLRotation;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	float FootLockLAlpha;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	FVector FootLockRLocation;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	FRotator FootLockRRotation;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	float FootLockRAlpha;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	FVector FootOffsetLLocation;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	FRotator FootOffsetLRotation;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	FVector FootOffsetRLocation;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	FRotator FootOffsetRRotation;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	FVector PelvisOffset;

	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Feet IK")
	float PelvisAlpha;

private:
	void UpdateFootIK(float deltaTime);

	void SetFootOffsets(float deltaTime, const FName& enableFootIKCurveName, const FName& iKFootBoneName, const FName& rootBoneName, FVector& currLocationTarget, FVector& currLocationOffset, FRotator& currRotationOffset);

	void SetPelvisIKOffset(float deltaTime, const FVector& footOffsetLTarget, const FVector& footOffsetRTarget);

	void SetFootLocking(float deltaTime, const FName& enableFootIKCurveName, const FName& footLockCurveName, const FName& iKFootBoneName, float& currentFootLockAlpha, FVector& currentFootLockLocation, FRotator& currentFootLockRotation);

	void SetFootLockOffsets(float deltaTime, FVector& localLocation, FRotator& localRotation);

	FVector FootOffsetLTarget;

	FVector FootOffsetRTarget;
#pragma endregion

#pragma region Swinging
protected:
	UPROPERTY(BlueprintReadOnly, Category="Swing")
	FRotator CurrThighRotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Swing")
	TObjectPtr<UCharacterAnimationSwingSettings> SwingSettings;

private:
	FRotator TargetRotation;

	void UpdateThighRotation(float deltaTime);
#pragma endregion
};
