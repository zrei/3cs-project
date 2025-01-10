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
	void AnimationUpdate();

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
	FName LeftFoot;

	UPROPERTY(EditAnywhere)
	FName RightFoot;

	UPROPERTY(EditAnywhere)
	FName LeftFootIKCurveName;

	UPROPERTY(EditAnywhere)
	float IKTraceDistanceAboveFoot;

	UPROPERTY(EditAnywhere)
	float IKTraceDistanceBelowFoot;

	UPROPERTY(EditAnywhere)
	float FootHeight;

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
	FVector FootOffsetRLocation;

	UPROPERTY(BlueprintReadOnly)
	FRotator FootOffsetRRotation;

	UPROPERTY(BlueprintReadOnly)
	FVector PelvisOffset;

	UPROPERTY(BlueprintReadOnly)
	float PelvisAlpha;

private:
	void SetupFootIK();

	void SetFootOffsets(float deltaTime, FName enableFootIKCurveName, FName IKFootBoneName, FName RootBoneName);

	void SetPelvisIKOffset();

	void SetFootLocking();

	void SetFootLockOffsets();
#pragma endregion
};
