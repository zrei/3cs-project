// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Animation/CharacterAnimationInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Data/Character/CharacterAnimationFeetIKSettings.h"
#include "Data/Character/CharacterAnimationLocomotionSettings.h"
#include "Data/Character/CharacterAnimationLookSettings.h"
#include "Data/Character/CharacterAnimationSwingSettings.h"

void UCharacterAnimationInstance::AnimationUpdate(float deltaTime)
{
	if (!IsRunningInGame)
		return;
	if (!CharacterRef)
		return;

	UpdateCharacterState();
	UpdateMovementState();
	UpdateStride();
	
	UpdateLookState(deltaTime);

	UpdateFootIK(deltaTime);

	UpdateHandIK();

	UpdateThighRotation(deltaTime);
}

void UCharacterAnimationInstance::InitializeCharacterReferences()
{
	TObjectPtr<APawn> owningPawn = TryGetPawnOwner();
	CharacterRef = Cast<ABase_MyCharacter>(owningPawn);

	if (CharacterRef)
	{
		CharacterMovementRef = CharacterRef->GetCharacterMovement();
	}

	SkeletalMesh = GetOwningComponent();
	IsRunningInGame = GetWorld()->GetGameInstance() != nullptr;
}

#pragma region Locomotion
void UCharacterAnimationInstance::UpdateMovementState()
{
	VelocityVector = CharacterMovementRef->Velocity;
	double velocity = UKismetMathLibrary::VSizeXY(VelocityVector);
	IsMoving = velocity > 0;
	IsSwinging = CharacterRef->GetCurrentState().CharacterMovementState == ECharacterMovementState::SWINGING;
	IsFalling = CharacterMovementRef->IsFalling();
	IsInAir = IsSwinging || IsFalling;
	CurrTurnBlendAlpha = IsMoving ? LocomotionSettings->MovingTurnBlendAlpha : LocomotionSettings->StationaryTurnBlendAlpha;
}

void UCharacterAnimationInstance::UpdateCharacterState()
{
	ECharacterMovementState prevMovementState = CurrMovementState;
	CurrMovementState = CharacterRef->GetCurrentState().CharacterMovementState;
	if (prevMovementState != CurrMovementState)
		OnCharacterMovementStateChanged(prevMovementState, CurrMovementState);
}

void UCharacterAnimationInstance::OnCharacterMovementStateChanged(ECharacterMovementState prevState, ECharacterMovementState currState)
{
	if (currState == ECharacterMovementState::JUMPING)
	{
		IsJumping = true;
		GetWorld()->GetTimerManager().SetTimer(JumpTimerHandle, this, &UCharacterAnimationInstance::ResetJumpState, LocomotionSettings->JumpTime, false);
	}
	else if (prevState == ECharacterMovementState::JUMPING)
	{
		GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);
		IsJumping = false;
	}
}

void UCharacterAnimationInstance::UpdateStride()
{
	Stride = CharacterRef->GetCurrentState().CurrCharacterSpeed / CharacterRef->GetMovementSettings().CharacterRunMovementSpeed;
	MovementSpeed = Stride;
}

void UCharacterAnimationInstance::ResetJumpState()
{
	IsJumping = false;
}
#pragma endregion

#pragma region Look
void UCharacterAnimationInstance::UpdateLookState(float deltaTime)
{
	if (IsSwinging)
	{
		LookYaw = FMath::FInterpTo(LookYaw, 0.5, deltaTime, LookSettings->RotatingLookYawInterpolationSpeed);
		LookPitch = FMath::FInterpTo(LookPitch, 0, deltaTime, LookSettings->LookPitchInterpolationSpeed);
		return;
	}
	
	bool isTurning = CharacterRef->GetCurrentState().CurrRotationDirection != ERotateDirection::NONE;
	float diff = CharacterRef->GetCurrentState().TargetCharacterRotation - CharacterRef->GetCurrentState().CurrCharacterRotation;
	float targetLookYaw = (FMath::Clamp(diff/90, -1, 1) + 1) / 2;
	LookYaw = FMath::FInterpTo(LookYaw, targetLookYaw, deltaTime, isTurning ? LookSettings->RotatingLookYawInterpolationSpeed : LookSettings->LookYawInterpolationSpeed);
	float targetLookPitch = FMath::Clamp(CharacterRef->GetCurrentState().CurrLookPitch, -90, 90);
	LookPitch = FMath::FInterpTo(LookPitch, targetLookPitch, deltaTime, LookSettings->LookPitchInterpolationSpeed);
}
#pragma endregion

#pragma region Feet IK
void UCharacterAnimationInstance::UpdateFootIK(float deltaTime)
{
	SetFootLocking(deltaTime, FeetIKSettings->LeftFootIKCurveName, FeetIKSettings->LeftFootLockCurve, FeetIKSettings->LeftIKFootBoneName, FootLockLAlpha, FootLockLLocation, FootLockLRotation);
	SetFootLocking(deltaTime, FeetIKSettings->RightFootIKCurveName, FeetIKSettings->RightFootLockCurve, FeetIKSettings->RightIKFootBoneName, FootLockRAlpha, FootLockRLocation, FootLockRRotation);
	SetFootOffsets(deltaTime, FeetIKSettings->LeftFootIKCurveName, FeetIKSettings->LeftIKFootBoneName, FeetIKSettings->LeftFootRootBoneName, FootOffsetLTarget, FootOffsetLLocation, FootOffsetLRotation);
	SetFootOffsets(deltaTime, FeetIKSettings->RightFootIKCurveName, FeetIKSettings->RightIKFootBoneName, FeetIKSettings->RightFootRootBoneName, FootOffsetRTarget, FootOffsetRLocation, FootOffsetRRotation);
	SetPelvisIKOffset(deltaTime, FootOffsetLTarget, FootOffsetRTarget);
}

void UCharacterAnimationInstance::SetFootOffsets(float deltaTime, const FName& enableFootIKCurveName, const FName& iKFootBoneName, const FName& rootBoneName, FVector& currLocationTarget, FVector& currLocationOffset, FRotator& currRotationOffset)
{
	if (GetCurveValue(enableFootIKCurveName) <= 0)
	{
		currLocationOffset = FVector::Zero();
		currRotationOffset = FRotator::ZeroRotator;
		return;
	}
	
	FVector iKFootBoneLocation = SkeletalMesh->GetSocketLocation(iKFootBoneName);
	FVector rootBoneLocation = SkeletalMesh->GetSocketLocation(rootBoneName);
	FVector iKFootFloorLocation{iKFootBoneLocation.X, iKFootBoneLocation.Y, rootBoneLocation.Z};

	FVector lineTraceStart{ iKFootFloorLocation.X, iKFootFloorLocation.Y, iKFootFloorLocation.Z + FeetIKSettings->IKTraceDistanceAboveFoot };
	FVector lineTraceEnd{ iKFootFloorLocation.X, iKFootFloorLocation.Y, iKFootFloorLocation.Z - FeetIKSettings->IKTraceDistanceBelowFoot };
	
	//initialize hit info
	FHitResult hitResult(ForceInit);

	FRotator targetRotationOffset;

	ETraceTypeQuery traceChannel = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), lineTraceStart, lineTraceEnd, traceChannel, false, {}, EDrawDebugTrace::None, hitResult, true))
	{
		if (CharacterMovementRef->IsWalkable(hitResult))
		{
			currLocationTarget = hitResult.ImpactNormal * FeetIKSettings->FootHeight + hitResult.ImpactPoint - (iKFootFloorLocation + FVector::UpVector * FeetIKSettings->FootHeight);
			double rotatorX = FMath::Atan2(hitResult.ImpactNormal.Y, hitResult.ImpactNormal.Z);
			double rotatorY = FMath::Atan2(hitResult.ImpactNormal.X, hitResult.ImpactNormal.Z) * -1;
			targetRotationOffset = {rotatorX, rotatorY, 0};
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

	currRotationOffset = UKismetMathLibrary::RInterpTo(currRotationOffset, targetRotationOffset, deltaTime, 30);
}

void UCharacterAnimationInstance::SetPelvisIKOffset(float deltaTime, const FVector& footOffsetLTarget, const FVector& footOffsetRTarget)
{
	PelvisAlpha = (GetCurveValue(FeetIKSettings->LeftFootIKCurveName) + GetCurveValue(FeetIKSettings->RightFootIKCurveName)) / 2;

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

void UCharacterAnimationInstance::SetFootLocking(float deltaTime, const FName& enableFootIKCurveName, const FName& footLockCurveName, const FName& iKFootBoneName, float& currentFootLockAlpha, FVector& currentFootLockLocation, FRotator& currentFootLockRotation)
{
	if (GetCurveValue(enableFootIKCurveName) <= 0)
	{
		return;
	}

	// disable foot locking if the character is moving
	if (IsMoving)
	{
		currentFootLockAlpha = 0;
		return;
	}

	float footLockCurveValue = GetCurveValue(footLockCurveName);

	if (footLockCurveValue >= 0.99 || footLockCurveValue < currentFootLockAlpha)
	{
		currentFootLockAlpha = footLockCurveValue;
	}

	if (currentFootLockAlpha >= 0.99)
	{
		FTransform socketTransform = SkeletalMesh->GetSocketTransform(iKFootBoneName, RTS_Component);
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

	FVector locationDifference = SkeletalMesh->GetComponentRotation().UnrotateVector(VelocityVector * deltaTime);

	localLocation = UKismetMathLibrary::RotateAngleAxis(localLocation - locationDifference, rotationDifference.Yaw, { 0, 0, -1 });
	
	localRotation = UKismetMathLibrary::NormalizedDeltaRotator(localRotation, rotationDifference);
}
#pragma endregion

#pragma region Swinging
void UCharacterAnimationInstance::UpdateThighRotation(float deltaTime)
{
	if (CharacterRef->GetCurrentState().CharacterMovementState != ECharacterMovementState::SWINGING || CharacterRef->GetCurrentState().RopeInputState == ERopeInputState::SHIMMY)
	{
		CurrThighRotation = FRotator::ZeroRotator;
		return;
	}

	FVector2D swingInput = {CharacterRef->GetCurrentState().MovementInput.Y, CharacterRef->GetCurrentState().MovementInput.X};
	TargetRotation = {-swingInput.Y * SwingSettings->MaxLegRotation, 0, -swingInput.X * SwingSettings->MaxLegRotation};
	CurrThighRotation = FMath::RInterpTo(CurrThighRotation, TargetRotation, deltaTime, SwingSettings->LegRotationInterpSpeed);
}
#pragma endregion

void UCharacterAnimationInstance::UpdateHandIK()
{
	//CharacterRef->GetMesh()->ComponentSpace
	LeftHandIKAlpha = CharacterRef->GetCurrentState().EnableHandIK ? 1.0f : 0.0f;
	RightHandIKAlpha = CharacterRef->GetCurrentState().EnableHandIK ? 1.0f : 0.0f;
	LeftHandPosition = CharacterRef->GetCurrentState().LeftHandPosition;
	RightHandPosition = CharacterRef->GetCurrentState().RightHandPosition;
}