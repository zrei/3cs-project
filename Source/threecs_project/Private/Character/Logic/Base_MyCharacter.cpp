// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Logic/Base_MyCharacter.h"
#include "Controller/MyPlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Data/Character/CharacterTurnAnimationSettings.h"
#include "Data/Character/CharacterLocomotionSettings.h"
#include "Data/Character/CharacterCameraSettings.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#pragma region Initialisation
// Sets default values
ABase_MyCharacter::ABase_MyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	CameraParent = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Parent"));
	CameraParent->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraParent);
}

// Called when the game starts or when spawned
void ABase_MyCharacter::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<USkeletalMeshComponent> skeletonMesh = FindComponentByClass<USkeletalMeshComponent>();
	if (skeletonMesh)
		MainAnimInstance = skeletonMesh->GetAnimInstance();

	CurrCharacterState.CharacterMovementState = ECharacterMovementState::IDLE;
	CurrCharacterState.CharacterGait = ECharacterGait::WALK;
	CurrCharacterState.CurrRotationDirection = ERotateDirection::NONE;
	CurrCharacterState.NextRotationDirection = ERotateDirection::NONE;
	CurrCharacterState.TargetCharacterSpeed = 0;
	CurrCharacterState.CurrCharacterSpeed = 0;
	CurrCharacterState.CurrCameraRotation = FRotator{0, GetActorRotation().Yaw, 0};
	CurrCharacterState.CurrCharacterRotation = CurrCharacterState.CurrCameraRotation;
	CurrCharacterState.TargetCharacterRotation = CurrCharacterState.CurrCharacterRotation;
	CurrCharacterState.NextTargetCharacterRotation = CurrCharacterState.CurrCharacterRotation;
	CurrCharacterState.EnableHandIK = false;
	CurrCharacterState.LeftHandPosition = FVector::Zero();
	CurrCharacterState.RightHandPosition = FVector::Zero();

	HasCameraInput = false;
	CameraInput = FVector2D::Zero();

	RotationCurveScaleValue = 1;
	RotationCountdownTimer = 0;

	CurrPlayingTurnSequence = nullptr;

	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	SubscribeToLocomotionInputs(controller);
	
	FInputActionWrapper& cameraMovementWrapper = controller->GetActionInputWrapper(FInputType::CAMERA_MOVEMENT);
	cameraMovementWrapper.ActionStartedEvent.AddUObject(this, &ABase_MyCharacter::OnCameraMovementStarted);
	cameraMovementWrapper.ActionTriggeredEvent.AddUObject(this, &ABase_MyCharacter::OnCameraMovementTriggered);
	cameraMovementWrapper.ActionCompletedEvent.AddUObject(this, &ABase_MyCharacter::OnCameraMovementComplete);
	
	RotateCamera();
}

void ABase_MyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	if (controller)
	{
		UnsubscribeToLocomotionInputs(controller);

		FInputActionWrapper& cameraMovementWrapper = controller->GetActionInputWrapper(FInputType::CAMERA_MOVEMENT);
		cameraMovementWrapper.ActionStartedEvent.RemoveAll(this);
		cameraMovementWrapper.ActionTriggeredEvent.RemoveAll(this);
		cameraMovementWrapper.ActionCompletedEvent.RemoveAll(this);
	}
}
#pragma endregion

#pragma region Locomotion
void ABase_MyCharacter::SubscribeToLocomotionInputs(AMyPlayerController* const playerController)
{
	FInputActionWrapper& locomotionMovementWrapper = playerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT);
	locomotionMovementWrapper.ActionStartedEvent.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementStarted);
	locomotionMovementWrapper.ActionTriggeredEvent.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementTriggered);
	locomotionMovementWrapper.ActionCompletedEvent.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementComplete);
	playerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionStartedEvent.AddUObject(this, &ABase_MyCharacter::OnCharacterJump);
	playerController->GetActionInputWrapper(FInputType::LOCOMOTION_GAIT).ActionStartedEvent.AddUObject(this, &ABase_MyCharacter::OnGaitChangeTriggered);
}

void ABase_MyCharacter::UnsubscribeToLocomotionInputs(AMyPlayerController* const playerController)
{
	FInputActionWrapper& locomotionMovementWrapper = playerController->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT);
	locomotionMovementWrapper.ActionStartedEvent.RemoveAll(this);
	locomotionMovementWrapper.ActionTriggeredEvent.RemoveAll(this);
	locomotionMovementWrapper.ActionCompletedEvent.RemoveAll(this);
	playerController->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.RemoveAll(this);
	playerController->GetActionInputWrapper(FInputType::LOCOMOTION_GAIT).ActionStartedEvent.RemoveAll(this);
}
#pragma endregion

#pragma region Movement
void ABase_MyCharacter::OnCharacterMovementStarted(const FInputActionInstance& _)
{
	if (CurrCharacterState.CharacterMovementState != ECharacterMovementState::JUMPING && CurrCharacterState.CharacterMovementState != ECharacterMovementState::SWINGING)
		CurrCharacterState.CharacterMovementState = ECharacterMovementState::MOVING;
	// reset gait to walk
	CurrCharacterState.CharacterGait = ECharacterGait::WALK;
	SetTargetCharacterMovementSpeed();
	StopCurrentlyPlayingTurningMontage();
}

void ABase_MyCharacter::OnCharacterMovementTriggered(const FInputActionInstance& inputActionInstance)
{
	CurrCharacterState.MovementInput = inputActionInstance.GetValue().Get<FVector2D>();
	CurrCharacterState.MovementInput.Normalize();
	SetTargetCharacterRotation();
	RotationCountdownTimer = 0;

	if (CurrCharacterState.NextTargetCharacterRotation.Yaw != CurrCharacterState.TargetCharacterRotation.Yaw)
	{
		StopCurrentlyPlayingTurningMontage();
	}
}

void ABase_MyCharacter::Move(float deltaTime)
{
	FRotator characterRotation{ 0, CurrCharacterState.CurrCameraRotation.Yaw, 0 };
	
	FVector forwardDirection = UKismetMathLibrary::GetForwardVector(characterRotation);
	float forwardMovementAmount = CurrCharacterState.MovementInput.Y * CurrCharacterState.CurrCharacterSpeed;
	AddMovementInput(forwardDirection, forwardMovementAmount);

	FVector rightDirection = UKismetMathLibrary::GetRightVector(characterRotation);
	float rightMovementAmount = CurrCharacterState.MovementInput.X * CurrCharacterState.CurrCharacterSpeed;
	AddMovementInput(rightDirection, rightMovementAmount);
}

void ABase_MyCharacter::OnCharacterMovementComplete(const FInputActionInstance& _)
{
	if (CurrCharacterState.CharacterMovementState != ECharacterMovementState::JUMPING && CurrCharacterState.CharacterMovementState != ECharacterMovementState::SWINGING)
		CurrCharacterState.CharacterMovementState = ECharacterMovementState::IDLE;
	CurrCharacterState.MovementInput = FVector2D::Zero();
	SetTargetCharacterMovementSpeed();
	SetTargetCharacterRotation();
	StopCurrentlyPlayingTurningMontage();	
}

void ABase_MyCharacter::SetCharacterMovementSpeed(float deltaTime)
{
	if (CurrCharacterState.TargetCharacterSpeed > CurrCharacterState.CurrCharacterSpeed)
	{
		float uncappedMovementSpeed = CurrCharacterState.CurrCharacterSpeed + GetMovementSettings().CharacterAcceleration * deltaTime;
		CurrCharacterState.CurrCharacterSpeed = FMath::Min(uncappedMovementSpeed, CurrCharacterState.TargetCharacterSpeed);
	}
	else if (CurrCharacterState.TargetCharacterSpeed < CurrCharacterState.CurrCharacterSpeed)
	{
		float uncappedMovementSpeed = CurrCharacterState.CurrCharacterSpeed - GetMovementSettings().CharacterDecceleration * deltaTime;
		CurrCharacterState.CurrCharacterSpeed = FMath::Max(uncappedMovementSpeed, CurrCharacterState.TargetCharacterSpeed);
	}
}

void ABase_MyCharacter::SetTargetCharacterMovementSpeed()
{
	if (CurrCharacterState.CharacterMovementState == ECharacterMovementState::IDLE)
		CurrCharacterState.TargetCharacterSpeed = 0;
	else
		CurrCharacterState.TargetCharacterSpeed = CurrCharacterState.CharacterGait == ECharacterGait::RUN ? GetMovementSettings().CharacterRunMovementSpeed : GetMovementSettings().CharacterWalkMovementSpeed;
}

float ABase_MyCharacter::GetMovementRotation() const
{
	const float dotProduct = FVector2D{ 0, 1 }.Dot(CurrCharacterState.MovementInput);
	float movementRotation = FMath::Acos(dotProduct) * (180 / PI);
	if (CurrCharacterState.MovementInput.X < 0)
		movementRotation = -movementRotation;
	return movementRotation;
}

void ABase_MyCharacter::OnMovementModeChanged(EMovementMode prevMovementMode, uint8 previousCustomMode)
{
	Super::OnMovementModeChanged(prevMovementMode, previousCustomMode);

	if (CurrCharacterState.CharacterMovementState == ECharacterMovementState::SWINGING)
		return;

	if (prevMovementMode == EMovementMode::MOVE_Falling)
	{
		if (CurrCharacterState.MovementInput != FVector2D::Zero())
			CurrCharacterState.CharacterMovementState = ECharacterMovementState::MOVING;
		else
			CurrCharacterState.CharacterMovementState = ECharacterMovementState::IDLE;
	}
}

void ABase_MyCharacter::OnCharacterJump(const FInputActionInstance& _)
{
	// cannot jump while already jumping
	if (CurrCharacterState.CharacterMovementState == ECharacterMovementState::JUMPING)
		return;

	// jump off rope if swinging
	if (CurrCharacterState.CharacterMovementState == ECharacterMovementState::SWINGING)
	{
		CurrCharacterState.CharacterMovementState = ECharacterMovementState::EXIT_SWINGING;
		return;
	}

	StopCurrentlyPlayingTurningMontage();
	Jump();
	CurrCharacterState.CharacterMovementState = ECharacterMovementState::JUMPING;
}
#pragma endregion

#pragma region Gait
void ABase_MyCharacter::OnGaitChangeTriggered(const FInputActionInstance& _)
{
	if (CurrCharacterState.CharacterMovementState == ECharacterMovementState::SWINGING)
	{
		CurrCharacterState.RopeInputState = CurrCharacterState.RopeInputState == ERopeInputState::SHIMMY ? ERopeInputState::SWING : ERopeInputState::SHIMMY;
		ControlSchemeChangedEvent.Broadcast(CurrCharacterState.RopeInputState == ERopeInputState::SHIMMY ? EControlScheme::ROPE_SHIMMY : EControlScheme::ROPE_SWING);
		return;
	}
	
	if (CurrCharacterState.CharacterMovementState != ECharacterMovementState::MOVING)
		return;
	CurrCharacterState.CharacterGait = CurrCharacterState.CharacterGait == ECharacterGait::RUN ? ECharacterGait::WALK : ECharacterGait::RUN;
	SetTargetCharacterMovementSpeed();
}
#pragma endregion

#pragma region Rotation
void ABase_MyCharacter::SetTargetCharacterRotation()
{
	if (CurrCharacterState.CharacterMovementState == ECharacterMovementState::MOVING || CurrCharacterState.CharacterMovementState == ECharacterMovementState::JUMPING)
	{
		float rotationFromMovementInput = GetMovementRotation();
		CurrCharacterState.NextTargetCharacterRotation = {0, FMath::ClampAngle(CurrCharacterState.CurrCameraRotation.Yaw + rotationFromMovementInput, -180, 179.9), 0};
	}
	else
	{
		CurrCharacterState.NextTargetCharacterRotation = {0, CurrCharacterState.CurrCameraRotation.Yaw, 0};
	}

	float diff = ABase_MyCharacter::CalculateShortestRotationDiff(CurrCharacterState.CurrCharacterRotation.Yaw, CurrCharacterState.NextTargetCharacterRotation.Yaw);
	if (diff > 0)
	{
		CurrCharacterState.NextRotationDirection = ERotateDirection::LEFT;
	}
	else if (diff < 0)
	{
		CurrCharacterState.NextRotationDirection = ERotateDirection::RIGHT;
	}
	else
	{
		CurrCharacterState.NextRotationDirection = ERotateDirection::NONE;
	}
}

void ABase_MyCharacter::UpdateCharacterMovingRotation(float deltaTime)
{
	CurrCharacterState.CurrCharacterRotation = UKismetMathLibrary::RInterpTo(CurrCharacterState.CurrCharacterRotation, CurrCharacterState.TargetCharacterRotation, deltaTime, GetMovementSettings().CharacterMovingRotationalSpeed);
}

bool ABase_MyCharacter::ShouldDoMontageRotation() const
{
	return FMath::Abs(ABase_MyCharacter::CalculateShortestRotationDiff(CurrCharacterState.CurrCharacterRotation.Yaw, CurrCharacterState.TargetCharacterRotation.Yaw)) >= GetMovementSettings().MontageRotationAngleThreshold;
}

bool ABase_MyCharacter::ShouldRotateInPlace() const
{
	return !IsPlayingTurningMontage() && CurrCharacterState.CharacterMovementState == ECharacterMovementState::IDLE && CurrCharacterState.CurrCharacterSpeed == 0 && ShouldDoMontageRotation();
}

bool ABase_MyCharacter::ShouldDoMovingRotation() const
{
	return !IsPlayingTurningMontage() && CurrCharacterState.CharacterMovementState == ECharacterMovementState::MOVING && FMath::Abs(ABase_MyCharacter::CalculateShortestRotationDiff(CurrCharacterState.CurrCharacterRotation.Yaw, CurrCharacterState.TargetCharacterRotation.Yaw)) >= GetMovementSettings().MovingRotationAngleThreshold;
}

void ABase_MyCharacter::SetTurnAnimationAsset()
{
	bool shouldDoBigRotation = FMath::Abs(ABase_MyCharacter::CalculateShortestRotationDiff(CurrCharacterState.CurrCharacterRotation.Yaw, CurrCharacterState.TargetCharacterRotation.Yaw)) > 90;
	if (CurrCharacterState.CurrRotationDirection == ERotateDirection::RIGHT && shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = NormalTurnAnimationSettings->TurnRightMoreThan180Asset;
	}
	else if (CurrCharacterState.CurrRotationDirection == ERotateDirection::RIGHT && !shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = NormalTurnAnimationSettings->TurnRightLessThan180Asset;
	}
	else if (CurrCharacterState.CurrRotationDirection == ERotateDirection::LEFT && shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = NormalTurnAnimationSettings->TurnLeftMoreThan180Asset;
	}
	else if (CurrCharacterState.CurrRotationDirection == ERotateDirection::LEFT && !shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = NormalTurnAnimationSettings->TurnLeftLessThan180Asset;
	}
	else
	{
		CurrPlayingTurnSequence = nullptr;
	}
}

void ABase_MyCharacter::SetRotationCurveScaleValue()
{
	float diff = FMath::Abs(ABase_MyCharacter::CalculateShortestRotationDiff(CurrCharacterState.CurrCharacterRotation.Yaw, CurrCharacterState.TargetCharacterRotation.Yaw));
	if (diff <= 90)
	{
		RotationCurveScaleValue = diff / ABase_MyCharacter::NinetyDegreeRotationCurveAmount * 1.1;
	}
	else
	{
		RotationCurveScaleValue = diff / ABase_MyCharacter::OneHundredEightyDegreeRotationCurveAmount * 1.1;
	}
}

void ABase_MyCharacter::PlayTurningMontage()
{
	SetTurnAnimationAsset();
	SetRotationCurveScaleValue();
	float playRate = 1;
	// scale play rate if moving to match movement speed
	if (CurrCharacterState.CharacterMovementState == ECharacterMovementState::MOVING)
	{
		playRate = FMath::Clamp(CurrCharacterState.CurrCharacterSpeed, GetMovementSettings().MinimumTurnMontageSpeed, GetMovementSettings().MaximumTurnMontageSpeed) * GetMovementSettings().MovingRotationSpeedPlayRateScale;
	}
	RotationCurveScaleValue *= playRate;
	MainAnimInstance->PlaySlotAnimationAsDynamicMontage(CurrPlayingTurnSequence, NormalTurnAnimationSettings->LegsSlotName, 0, 0, playRate, 1, -1, CurrCharacterState.CharacterMovementState == ECharacterMovementState::MOVING ? MovingTurnStartTime : 0);
}

void ABase_MyCharacter::StopCurrentlyPlayingTurningMontage()
{
	MainAnimInstance->StopSlotAnimation(0.25, NormalTurnAnimationSettings->LegsSlotName);
}

void ABase_MyCharacter::UpdateCharacterRotationThroughCurve(float deltaTime)
{
	if (MainAnimInstance)
	{
		float currFrameRate = 1 / deltaTime;
		float rotationCurveValue = FMath::Abs(MainAnimInstance->GetCurveValue(CharacterLocomotionSettings->RotationCurveName) * RotationCurveScaleValue * (TurnAnimationTargetFrameRate / currFrameRate));		
				
		if (CurrCharacterState.CurrRotationDirection == ERotateDirection::RIGHT)
		{
			float uncappedHorizontalAngle = FMath::ClampAngle(CurrCharacterState.CurrCharacterRotation.Yaw - rotationCurveValue, -180, 179.9);
			if ((uncappedHorizontalAngle > 0 && CurrCharacterState.TargetCharacterRotation.Yaw < 0) || (uncappedHorizontalAngle < 0 && CurrCharacterState.TargetCharacterRotation.Yaw > 0))
			{
				CurrCharacterState.CurrCharacterRotation = { 0, uncappedHorizontalAngle, 0 };
		}
			else
			{
				CurrCharacterState.CurrCharacterRotation = { 0, FMath::Max(uncappedHorizontalAngle, CurrCharacterState.TargetCharacterRotation.Yaw), 0 };
			}
		}
		else if (CurrCharacterState.CurrRotationDirection == ERotateDirection::LEFT)
		{
			float uncappedHorizontalAngle = FMath::ClampAngle(CurrCharacterState.CurrCharacterRotation.Yaw + rotationCurveValue, -180, 179.9);
			if ((uncappedHorizontalAngle > 0 && CurrCharacterState.TargetCharacterRotation.Yaw < 0) || (uncappedHorizontalAngle < 0 && CurrCharacterState.TargetCharacterRotation.Yaw > 0))
			{
				CurrCharacterState.CurrCharacterRotation = { 0, uncappedHorizontalAngle, 0 };
			}
			else
			{
				CurrCharacterState.CurrCharacterRotation = { 0, FMath::Min(uncappedHorizontalAngle, CurrCharacterState.TargetCharacterRotation.Yaw), 0 };
			}
		}
	}
}

bool ABase_MyCharacter::IsPlayingTurningMontage() const
{
	return CurrPlayingTurnSequence && MainAnimInstance->IsPlayingSlotAnimation(CurrPlayingTurnSequence, NormalTurnAnimationSettings->LegsSlotName);
}
#pragma endregion

#pragma region Camera
const FCameraSettings& ABase_MyCharacter::GetCameraSettings() const
{
	return CharacterCameraSettings->CameraSettings;
}

void ABase_MyCharacter::OnCameraMovementStarted(const FInputActionInstance& _)
{
	HasCameraInput = true;
}

void ABase_MyCharacter::OnCameraMovementTriggered(const FInputActionInstance& inputActionInstance)
{
	CameraInput = inputActionInstance.GetValue().Get<FVector2D>();
}

void ABase_MyCharacter::OnCameraMovementComplete(const FInputActionInstance& _)
{
	HasCameraInput = false;
}

void ABase_MyCharacter::SetTargetCameraRotation(float deltaTime)
{
	CurrCharacterState.CurrCameraRotation.Pitch = FMath::ClampAngle(CurrCharacterState.CurrCameraRotation.Pitch + CameraInput.Y * GetCameraSettings().CameraRotationalSpeed * deltaTime, GetCameraSettings().MinViewVerticalAngle, GetCameraSettings().MaxViewVerticalAngle);
	CurrCharacterState.CurrCameraRotation.Yaw = FMath::ClampAngle(CurrCharacterState.CurrCameraRotation.Yaw + CameraInput.X * GetCameraSettings().CameraRotationalSpeed * deltaTime, -180, 179.9);
}
#pragma endregion

#pragma region Movement and Rotation Info
const FCharacterMovementSettings& ABase_MyCharacter::GetMovementSettings() const
{
	return CharacterLocomotionSettings->MovementSettings;
}

const FCharacterState& ABase_MyCharacter::GetCurrentState() const
{
	return CurrCharacterState;
}
#pragma endregion

#pragma region Rope Swing
bool ABase_MyCharacter::EnterSwingState()
{
	if (CurrCharacterState.CharacterMovementState == ECharacterMovementState::SWINGING)
		return false;
	
	CurrCharacterState.CharacterMovementState = ECharacterMovementState::SWINGING;
	CurrCharacterState.EnableHandIK = true;

	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
	GetCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	AMyPlayerController* playerController = Cast<AMyPlayerController>(GetController());

	StopCurrentlyPlayingTurningMontage();
	ControlSchemeChangedEvent.Broadcast(EControlScheme::ROPE_SWING);
	return true;
}

bool ABase_MyCharacter::ExitSwingState()
{
	if (CurrCharacterState.CharacterMovementState != ECharacterMovementState::EXIT_SWINGING)
		return false;

	CurrCharacterState.CharacterMovementState = ECharacterMovementState::JUMPING;
	CurrCharacterState.RopeInputState = ERopeInputState::SWING;
	CurrCharacterState.EnableHandIK = false;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Falling;

	AMyPlayerController* playerController = Cast<AMyPlayerController>(GetController());

	CurrCharacterState.MovementInput = FVector2D::Zero();

	FRotator currWorldRotation = GetActorRotation();
	CurrCharacterState.CurrCharacterRotation = FRotator{0, currWorldRotation.Yaw, 0};
	CurrCharacterState.CurrCameraRotation = CurrCharacterState.CurrCharacterRotation;
	SetActorRotation(CurrCharacterState.CurrCharacterRotation);

	ControlSchemeChangedEvent.Broadcast(EControlScheme::NORMAL);

	return true;
}
#pragma endregion

void ABase_MyCharacter::Tick(float deltaTime)
{
	if (HasCameraInput)
	{
		SetTargetCameraRotation(deltaTime);
		SetTargetCharacterRotation();
	}

	if (CurrCharacterState.CharacterMovementState != ECharacterMovementState::JUMPING)
	{
		if (IsPlayingTurningMontage())
		{
			UpdateCharacterRotationThroughCurve(deltaTime);
			SetActorRotation(CurrCharacterState.CurrCharacterRotation);
		}
		else
		{
			CurrCharacterState.TargetCharacterRotation = CurrCharacterState.NextTargetCharacterRotation;
			CurrCharacterState.CurrRotationDirection = CurrCharacterState.NextRotationDirection;
			if (ShouldDoMovingRotation())
			{
				if (ShouldDoMontageRotation())
				{
					PlayTurningMontage();
				}
				else
				{
					UpdateCharacterMovingRotation(deltaTime);
					SetActorRotation(CurrCharacterState.CurrCharacterRotation);
				}
			}
			else if (ShouldRotateInPlace())
			{
				// TODO: Clean this up
				RotationCountdownTimer += deltaTime;
				if (RotationCountdownTimer >= GetMovementSettings().RotationLookTimeThreshold)
				{
					RotationCountdownTimer = 0;
					PlayTurningMontage();
				}
			}
			else
			{
				RotationCountdownTimer = 0;
			}
		}

		SetCharacterMovementSpeed(deltaTime);
	}
	else
	{
		CurrCharacterState.TargetCharacterRotation = CurrCharacterState.NextTargetCharacterRotation;
		CurrCharacterState.CurrRotationDirection = CurrCharacterState.NextRotationDirection;
		UpdateCharacterMovingRotation(deltaTime);
		SetActorRotation(CurrCharacterState.CurrCharacterRotation);
	}

	if (CurrCharacterState.CharacterMovementState != ECharacterMovementState::SWINGING)
		Move(deltaTime);

	RotateCamera();
}

void ABase_MyCharacter::UpdateHandPositions(FVector left, FVector right)
{
	CurrCharacterState.LeftHandPosition = left;
	CurrCharacterState.RightHandPosition = right;
}
