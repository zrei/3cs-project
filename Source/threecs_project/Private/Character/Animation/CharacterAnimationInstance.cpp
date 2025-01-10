// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Animation/CharacterAnimationInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

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

	SkeletalMesh = GetOwningComponent();
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

void UCharacterAnimationInstance::SetFootOffsets(float deltaTime, FName enableFootIKCurveName, FName IKFootBoneName, FName RootBoneName, FVector& currLocationTarget, FVector& currLocationOffset, FRotator& currRotationOffset)
{
	if (GetCurveValue(enableFootIKCurveName) <= 0)
	{
		currLocationOffset = FVector::Zero();
		currRotationOffset = FRotator::ZeroRotator;
		return;
	}
	
	FVector IKFootBoneLocation = SkeletalMesh->GetSocketLocation(IKFootBoneName);
	FVector RootBoneLocation = SkeletalMesh->GetSocketLocation(RootBoneName);
	FVector IKFootFloorLocation{IKFootBoneLocation.X, IKFootBoneLocation.Y, RootBoneLocation.Z};

	FVector LineTraceStart{ IKFootFloorLocation.X, IKFootFloorLocation.Y, IKFootFloorLocation.Z + IKTraceDistanceAboveFoot };
	FVector LineTraceEnd{ IKFootFloorLocation.X, IKFootFloorLocation.Y, IKFootFloorLocation.Z - IKTraceDistanceBelowFoot };
	
	//Re-initialize hit info
	FHitResult RV_Hit(ForceInit);

	FVector ImpactPoint;
	FVector ImpactNormal;
	FRotator TargetRotationOffset;

	ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), LineTraceStart, LineTraceEnd, TraceChannel, false, {}, EDrawDebugTrace::None, RV_Hit, true, FColor::Red, FColor::Green, 500))
	{
		if (CharacterMovementRef->IsWalkable(RV_Hit))
		{
			ImpactPoint = RV_Hit.ImpactPoint;
			ImpactNormal = RV_Hit.ImpactNormal;

			currLocationTarget = ImpactNormal * FootHeight + ImpactPoint - (IKFootFloorLocation + FVector::UpVector * FootHeight);
			double rotatorX = FMath::Atan2(ImpactNormal.Y, ImpactNormal.Z);
			double rotatorY = FMath::Atan2(ImpactNormal.Z, ImpactNormal.X) * -1;
			TargetRotationOffset = {rotatorX, rotatorY, 0};
		}
	}

	if (currLocationOffset.Z > currLocationTarget.Z)
	{
		currLocationOffset = UKismetMathLibrary::VInterpTo(currLocationOffset, currLocationTarget, deltaTime, 30);
	}
	else
	{
		currLocationOffset = UKismetMathLibrary::VInterpTo(currLocationOffset, currLocationTarget, deltaTime, 15);
	}
}