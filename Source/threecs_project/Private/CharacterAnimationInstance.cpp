// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterAnimationInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UCharacterAnimationInstance::AnimationUpdate()
{
	if (!CharacterRef)
		return;

	UpdateCharacterState();
	UpdateMovementState();
	UpdateStride();
	UpdateInPlaceRotation();
}

void UCharacterAnimationInstance::InitializeCharacterReferences()
{
	TObjectPtr<APawn> owningPawn = TryGetPawnOwner();
	CharacterRef = Cast<ABase_MyCharacter>(owningPawn);

	if (CharacterRef)
	{
		CharacterMovementSettings = CharacterRef->GetMovementSettings();

		CharacterMovementRef = CharacterRef->GetCharacterMovement();
	}
}

void UCharacterAnimationInstance::UpdateMovementState()
{
	if (CharacterRef)
	{
		double velocity = UKismetMathLibrary::VSizeXY(CharacterMovementRef->Velocity);
		IsMoving = velocity > 0;
		IsFalling = CharacterMovementRef->IsFalling();
	}
}

void UCharacterAnimationInstance::UpdateCharacterState()
{
	if (CharacterRef)
		CurrCharacterState = CharacterRef->GetCurrentState();
}

void UCharacterAnimationInstance::UpdateStride()
{
	if (CharacterRef)
	{
		Stride = CurrCharacterState.CurrCharacterSpeed / CharacterMovementSettings.CharacterRunMovementSpeed;
		MovementSpeed = Stride;
	}
}

void UCharacterAnimationInstance::UpdateInPlaceRotation()
{
	if (!CharacterRef)
		return;

	if (CharacterRef->ShouldRotateInPlace() && !IsMoving)
	{
		switch (CurrCharacterState.RotationDirection)
		{
		case ERotateDirection::NONE:
			SetRotateValues(false, false);
			break;
		case ERotateDirection::LEFT:
			SetRotateValues(true, false);
			break;
		case ERotateDirection::RIGHT:
			SetRotateValues(false, true);
			break;
		}
	}
	else
	{
		SetRotateValues(false, false);
	}
}

void UCharacterAnimationInstance::SetRotateValues(bool rotateL, bool rotateR)
{
	RotateLeft = rotateL;
	RotateRight = rotateR;
}
