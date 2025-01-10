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
};
