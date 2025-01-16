// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Animation/CharacterAnimationInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void UCharacterAnimationInstance::AnimationUpdate(float deltaTime)
{
	if (!CharacterRef)
		return;

	UpdateCharacterState();
	UpdateMovementState();
	UpdateStride();

	UpdateFootIK(deltaTime);
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
		VelocityVector = CharacterMovementRef->Velocity;
		double velocity = UKismetMathLibrary::VSizeXY(VelocityVector);
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

#pragma region Feet IK
void UCharacterAnimationInstance::UpdateFootIK(float deltaTime)
{
	SetFootLocking(deltaTime, LeftFootIKCurveName, LeftFootLockCurve, LeftIKFootBoneName, FootLockLAlpha, FootLockLLocation, FootLockLRotation);
	SetFootLocking(deltaTime, RightFootIKCurveName, RightFootLockCurve, RightIKFootBoneName, FootLockRAlpha, FootLockRLocation, FootLockRRotation);
	SetFootOffsets(deltaTime, LeftFootIKCurveName, LeftIKFootBoneName, LeftFootRootBoneName, FootOffsetLTarget, FootOffsetLLocation, FootOffsetLRotation);
	SetFootOffsets(deltaTime, RightFootIKCurveName, RightIKFootBoneName, RightFootRootBoneName, FootOffsetRTarget, FootOffsetRLocation, FootOffsetRRotation);
	SetPelvisIKOffset(deltaTime, FootOffsetLTarget, FootOffsetRTarget);
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
	FVector rootBoneLocation = SkeletalMesh->GetSocketLocation(RootBoneName);
	FVector IKFootFloorLocation{IKFootBoneLocation.X, IKFootBoneLocation.Y, rootBoneLocation.Z};

	FVector LineTraceStart{ IKFootFloorLocation.X, IKFootFloorLocation.Y, IKFootFloorLocation.Z + IKTraceDistanceAboveFoot };
	FVector LineTraceEnd{ IKFootFloorLocation.X, IKFootFloorLocation.Y, IKFootFloorLocation.Z - IKTraceDistanceBelowFoot };
	
	//Re-initialize hit info
	FHitResult RV_Hit(ForceInit);

	FVector ImpactPoint;
	FVector ImpactNormal;
	FRotator TargetRotationOffset;

	ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), LineTraceStart, LineTraceEnd, TraceChannel, false, {}, EDrawDebugTrace::ForOneFrame, RV_Hit, true, FColor::Red, FColor::Green, 5))
	{
		if (CharacterMovementRef->IsWalkable(RV_Hit))
		{
			ImpactPoint = RV_Hit.ImpactPoint;
			ImpactNormal = RV_Hit.ImpactNormal;

			UE_LOG(LogTemp, Warning, TEXT("Up vector: %f, %f, %f"), FVector::UpVector.X, FVector::UpVector.Y, FVector::UpVector.Z);
			currLocationTarget = ImpactNormal * FootHeight + ImpactPoint - (IKFootFloorLocation + FVector::UpVector * FootHeight);
			double rotatorX = FMath::Atan2(ImpactNormal.Y, ImpactNormal.Z);
			double rotatorY = FMath::Atan2(ImpactNormal.X, ImpactNormal.Z) * -1;
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

	currRotationOffset = UKismetMathLibrary::RInterpTo(currRotationOffset, TargetRotationOffset, deltaTime, 30);
}

void UCharacterAnimationInstance::SetPelvisIKOffset(float deltaTime, FVector footOffsetLTarget, FVector footOffsetRTarget)
{
	PelvisAlpha = (GetCurveValue(LeftFootIKCurveName) + GetCurveValue(RightFootIKCurveName)) / 2;

	if (PelvisAlpha <= 0)
	{
		PelvisOffset = FVector::Zero();
		return;
	}

	FVector pelvisTarget;

	if (footOffsetLTarget.Z < footOffsetRTarget.Z)
	{
		pelvisTarget = footOffsetLTarget;
	}
	else
	{
		pelvisTarget = footOffsetRTarget;
	}

	if (pelvisTarget.Z > PelvisOffset.Z)
	{
		PelvisOffset = UKismetMathLibrary::VInterpTo(PelvisOffset, pelvisTarget, deltaTime, 10);
	}
	else
	{
		PelvisOffset = UKismetMathLibrary::VInterpTo(PelvisOffset, pelvisTarget, deltaTime, 15);
	}
}

void UCharacterAnimationInstance::SetFootLocking(float deltaTime, FName enableFootIKCurveName, FName footLockCurveName, FName IKFootBoneName, float& currentFootLockAlpha, FVector& currentFootLockLocation, FRotator& currentFootLockRotation)
{
	if (GetCurveValue(enableFootIKCurveName) <= 0)
	{
		return;
	}

	float footLockCurveValue = GetCurveValue(footLockCurveName);

	if (footLockCurveValue >= 0.99 || footLockCurveValue < currentFootLockAlpha)
	{
		currentFootLockAlpha = footLockCurveValue;
	}

	if (currentFootLockAlpha >= 0.99)
	{
		FTransform socketTransform = SkeletalMesh->GetSocketTransform(IKFootBoneName, RTS_Component);
		currentFootLockLocation = socketTransform.GetLocation();
		currentFootLockRotation = socketTransform.Rotator();
	}

	if (currentFootLockAlpha > 0)
	{
		SetFootLockOffsets(deltaTime, currentFootLockLocation, currentFootLockRotation);
	}
}

void UCharacterAnimationInstance::SetFootLockOffsets(float deltaTime, FVector& localLocation, FRotator& localRotation)
{
	FRotator rotationDifference;
	if (CharacterMovementRef->IsMovingOnGround())
	{
		rotationDifference = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRef->GetActorRotation(), CharacterMovementRef->GetLastUpdateRotation());
	}

	FVector locationDifference = UKismetMathLibrary::LessLess_VectorRotator(VelocityVector * deltaTime, SkeletalMesh->GetComponentRotation());

	localLocation = UKismetMathLibrary::RotateAngleAxis(localLocation - locationDifference, rotationDifference.Yaw, { 0, 0, -1 });
	
	localRotation = UKismetMathLibrary::NormalizedDeltaRotator(localRotation, rotationDifference);
}
#pragma endregion