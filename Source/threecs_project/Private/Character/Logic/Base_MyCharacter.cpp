// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Logic/Base_MyCharacter.h"
#include "Controller/MyPlayerController.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "InputMappingContext.h"

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

	MovementSettings.CharacterAcceleration = 0.2;
	MovementSettings.CharacterDecceleration = 0.3;
	MovementSettings.CharacterRunMovementSpeed = 0.7;
	MovementSettings.CharacterWalkMovementSpeed = 0.3;
	MovementSettings.CharacterStationaryRotationalSpeed = 15;
	MovementSettings.CharacterMovingRotationalSpeed = 70;
	MovementSettings.MovingRotationTime = 0.5;
	MovementSettings.RotationAngleThreshold = 45;
	MovementSettings.RotationLookTimeThreshold = 1;

	CameraSettings.CameraRotationalSpeed = 10;
	CameraSettings.MaxViewVerticalAngle = 30;
	CameraSettings.MinViewVerticalAngle = -60;

	RotationCurveName = FName{ "RotationAmount" };
	LegsSlotName = FName{ "Legs" };
}

// Called when the game starts or when spawned
void ABase_MyCharacter::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<USkeletalMeshComponent> skeletonMesh = FindComponentByClass<USkeletalMeshComponent>();
	if (skeletonMesh)
		MainAnimInstance = skeletonMesh->GetAnimInstance();

	CurrCharacterMovementState = ECharacterMovementState::IDLE;
	CurrCharacterGait = ECharacterGait::WALK;
	CurrRotationDirection = ERotateDirection::NONE;
	NextRotationDirection = ERotateDirection::NONE;
	TargetCharacterMovementSpeed = 0;
	CurrCharacterMovementSpeed = 0;

	HasCameraInput = false;
	CameraInput = FVector2D::Zero();
	CurrViewVerticalAngle = 0;

	CurrViewHorizontalAngle = GetActorRotation().Yaw;
	CurrCharacterHorizontalAngle = CurrViewHorizontalAngle;
	CurrTargetCharacterHorizontalAngle = CurrViewHorizontalAngle;
	NextTargetCharacterHorizontalAngle = CurrTargetCharacterHorizontalAngle;
	RotationCurveScaleValue = 1;
	RotationCountdownTimer = 0;

	CurrPlayingTurnSequence = nullptr;

	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	FInputActionWrapper& locomotionMovementWrapper = controller->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT);
	locomotionMovementWrapper.ActionStartedEvent.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementStarted);
	locomotionMovementWrapper.ActionTriggeredEvent.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementTriggered);
	locomotionMovementWrapper.ActionCompletedEvent.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementComplete);
	
	FInputActionWrapper& cameraMovementWrapper = controller->GetActionInputWrapper(FInputType::CAMERA_MOVEMENT);
	cameraMovementWrapper.ActionStartedEvent.AddUObject(this, &ABase_MyCharacter::OnCameraMovementStarted);
	cameraMovementWrapper.ActionTriggeredEvent.AddUObject(this, &ABase_MyCharacter::OnCameraMovementTriggered);
	cameraMovementWrapper.ActionCompletedEvent.AddUObject(this, &ABase_MyCharacter::OnCameraMovementComplete);
	
	controller->GetActionInputWrapper(FInputType::LOCOMOTION_GAIT).ActionStartedEvent.AddUObject(this, &ABase_MyCharacter::OnGaitChangeTriggered);

	controller->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.AddUObject(this, &ABase_MyCharacter::OnCharacterJump);

	RotateCamera();
}

void ABase_MyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	if (controller)
	{
		FInputActionWrapper& locomotionMovementWrapper = controller->GetActionInputWrapper(FInputType::LOCOMOTION_MOVEMENT);
		locomotionMovementWrapper.ActionStartedEvent.RemoveAll(this);
		locomotionMovementWrapper.ActionTriggeredEvent.RemoveAll(this);
		locomotionMovementWrapper.ActionCompletedEvent.RemoveAll(this);

		FInputActionWrapper& cameraMovementWrapper = controller->GetActionInputWrapper(FInputType::CAMERA_MOVEMENT);
		cameraMovementWrapper.ActionStartedEvent.RemoveAll(this);
		cameraMovementWrapper.ActionTriggeredEvent.RemoveAll(this);
		cameraMovementWrapper.ActionCompletedEvent.RemoveAll(this);

		controller->GetActionInputWrapper(FInputType::LOCOMOTION_GAIT).ActionStartedEvent.RemoveAll(this);

		controller->GetActionInputWrapper(FInputType::LOCOMOTION_JUMPING).ActionTriggeredEvent.RemoveAll(this);
	}
}
#pragma endregion

#pragma region Movement
void ABase_MyCharacter::OnCharacterMovementStarted(const FInputActionInstance& _)
{
	if (CurrCharacterMovementState != ECharacterMovementState::JUMPING)
		CurrCharacterMovementState = ECharacterMovementState::MOVING;
	// reset gait to walk
	CurrCharacterGait = ECharacterGait::WALK;
	SetTargetCharacterMovementSpeed();
	StopCurrentlyPlayingTurningMontage();
}

void ABase_MyCharacter::OnCharacterMovementTriggered(const FInputActionInstance& inputActionInstance)
{
	MovementInput = inputActionInstance.GetValue().Get<FVector2D>();
	MovementInput.Normalize();
	SetTargetCharacterRotation();
	RotationCountdownTimer = 0;

	if (NextTargetCharacterHorizontalAngle != CurrTargetCharacterHorizontalAngle)
	{
		StopCurrentlyPlayingTurningMontage();
	}
}

void ABase_MyCharacter::Move(float deltaTime)
{
	FRotator characterRotation{ 0, CurrViewHorizontalAngle, 0 };
	
	FVector forwardDirection = UKismetMathLibrary::GetForwardVector(characterRotation);
	float forwardMovementAmount = MovementInput.Y * CurrCharacterMovementSpeed;
	AddMovementInput(forwardDirection, forwardMovementAmount);

	FVector rightDirection = UKismetMathLibrary::GetRightVector(characterRotation);
	float rightMovementAmount = MovementInput.X * CurrCharacterMovementSpeed;
	AddMovementInput(rightDirection, rightMovementAmount);
}

void ABase_MyCharacter::OnCharacterMovementComplete(const FInputActionInstance& _)
{
	if (CurrCharacterMovementState != ECharacterMovementState::JUMPING)
		CurrCharacterMovementState = ECharacterMovementState::IDLE;
	MovementInput = FVector2D::Zero();
	SetTargetCharacterMovementSpeed();
	SetTargetCharacterRotation();
	StopCurrentlyPlayingTurningMontage();	
}

void ABase_MyCharacter::SetCharacterMovementSpeed(float deltaTime)
{
	if (TargetCharacterMovementSpeed > CurrCharacterMovementSpeed)
	{
		float uncappedMovementSpeed = CurrCharacterMovementSpeed + MovementSettings.CharacterAcceleration * deltaTime;
		CurrCharacterMovementSpeed = FMath::Min(uncappedMovementSpeed, TargetCharacterMovementSpeed);
	}
	else if (TargetCharacterMovementSpeed < CurrCharacterMovementSpeed)
	{
		float uncappedMovementSpeed = CurrCharacterMovementSpeed - MovementSettings.CharacterDecceleration * deltaTime;
		CurrCharacterMovementSpeed = FMath::Max(uncappedMovementSpeed, TargetCharacterMovementSpeed);
	}
}

void ABase_MyCharacter::SetTargetCharacterMovementSpeed()
{
	if (CurrCharacterMovementState == ECharacterMovementState::IDLE)
		TargetCharacterMovementSpeed = 0;
	else
		TargetCharacterMovementSpeed = CurrCharacterGait == ECharacterGait::RUN ? MovementSettings.CharacterRunMovementSpeed : MovementSettings.CharacterWalkMovementSpeed;
}

float ABase_MyCharacter::GetMovementRotation() const
{
	float dotProduct = FVector2D{ 0, 1 }.Dot(MovementInput);
	float movementRotation = FMath::Acos(dotProduct) * (180 / PI);
	if (MovementInput.X < 0)
		movementRotation = -movementRotation;
	return movementRotation;
}

void ABase_MyCharacter::OnMovementModeChanged(EMovementMode prevMovementMode, uint8 previousCustomMode)
{
	Super::OnMovementModeChanged(prevMovementMode, previousCustomMode);

	if (prevMovementMode == EMovementMode::MOVE_Falling)
	{
		if (MovementInput != FVector2D::Zero())
			CurrCharacterMovementState = ECharacterMovementState::MOVING;
		else
			CurrCharacterMovementState = ECharacterMovementState::IDLE;
	}
}

void ABase_MyCharacter::OnCharacterJump(const FInputActionInstance& _)
{
	// cannot jump while already jumping
	if (CurrCharacterMovementState == ECharacterMovementState::JUMPING)
		return;
	StopCurrentlyPlayingTurningMontage();
	Jump();
	CurrCharacterMovementState = ECharacterMovementState::JUMPING;
}
#pragma endregion

#pragma region Gait
void ABase_MyCharacter::OnGaitChangeTriggered(const FInputActionInstance& _)
{
	if (CurrCharacterMovementState != ECharacterMovementState::MOVING)
		return;
	CurrCharacterGait = CurrCharacterGait == ECharacterGait::RUN ? ECharacterGait::WALK : ECharacterGait::RUN;
	SetTargetCharacterMovementSpeed();
}
#pragma endregion

#pragma region Rotation
const float ABase_MyCharacter::NinetyDegreeRotationCurveAmount = 180;
const float ABase_MyCharacter::OneHundredEightyDegreeRotationCurveAmount = 360;

void ABase_MyCharacter::SetTargetCharacterRotation()
{
	if (CurrCharacterMovementState == ECharacterMovementState::MOVING)
	{
		float rotationFromMovementInput = GetMovementRotation();
		NextTargetCharacterHorizontalAngle = CurrViewHorizontalAngle + rotationFromMovementInput;
	}
	else
	{
		NextTargetCharacterHorizontalAngle = CurrViewHorizontalAngle;
	}
	
	// set target character horizontal angle to be from 0 - 360
	while (NextTargetCharacterHorizontalAngle < 0)
	{
		NextTargetCharacterHorizontalAngle += 360;
	}
	while (NextTargetCharacterHorizontalAngle > 360)
	{
		NextTargetCharacterHorizontalAngle -= 360;
	}
	
	float diff = CurrCharacterHorizontalAngle - NextTargetCharacterHorizontalAngle;
	
	if (FMath::Abs(diff) > 180)
	{
		// reverse the direction of the curr angle if diff > 180
		if (diff > 0)
		{
			ConvertRotation(CurrCharacterHorizontalAngle);
		}
		// reverse the direction of the target angle if diff < 180
		else
		{
			ConvertRotation(NextTargetCharacterHorizontalAngle);
		}
	}

	diff = CurrCharacterHorizontalAngle - NextTargetCharacterHorizontalAngle;

	if (CurrCharacterHorizontalAngle > NextTargetCharacterHorizontalAngle)
	{
		NextRotationDirection = ERotateDirection::RIGHT;
	}
	else if (CurrCharacterHorizontalAngle < NextTargetCharacterHorizontalAngle)
	{
		NextRotationDirection = ERotateDirection::LEFT;
	}
	else
	{
		NextRotationDirection = ERotateDirection::NONE;
	}
}

void ABase_MyCharacter::UpdateCharacterMovingRotation(float deltaTime)
{
	// lerp the rotation of the character towards the target horizontal angle
	// TODO: Handle small differences and large differences
	if (CurrRotationDirection == ERotateDirection::RIGHT)
	{
		float uncappedHorizontalAngle = CurrCharacterHorizontalAngle - MovementSettings.CharacterMovingRotationalSpeed * deltaTime;
		CurrCharacterHorizontalAngle = FMath::Max(uncappedHorizontalAngle, CurrTargetCharacterHorizontalAngle);
	}
	else if (CurrRotationDirection == ERotateDirection::LEFT)
	{
		float uncappedHorizontalAngle = CurrCharacterHorizontalAngle + MovementSettings.CharacterMovingRotationalSpeed * deltaTime;
		CurrCharacterHorizontalAngle = FMath::Min(uncappedHorizontalAngle, CurrTargetCharacterHorizontalAngle);
	}
}

bool ABase_MyCharacter::ShouldDoMontageRotation() const
{
	return FMath::Abs(CurrTargetCharacterHorizontalAngle - CurrCharacterHorizontalAngle) >= MovementSettings.RotationAngleThreshold;
}

bool ABase_MyCharacter::ShouldRotateInPlace() const
{
	return !IsPlayingTurningMontage() && CurrCharacterMovementState == ECharacterMovementState::IDLE && CurrCharacterMovementSpeed == 0 && ShouldDoMontageRotation();
}

bool ABase_MyCharacter::ShouldDoMovingRotation() const
{
	return !IsPlayingTurningMontage() && CurrCharacterMovementState == ECharacterMovementState::MOVING && CurrTargetCharacterHorizontalAngle != CurrCharacterHorizontalAngle;
}

void ABase_MyCharacter::SetTurnAnimationAsset()
{
	bool shouldDoBigRotation = FMath::Abs(CurrTargetCharacterHorizontalAngle - CurrCharacterHorizontalAngle) > 90;
	if (CurrRotationDirection == ERotateDirection::RIGHT && shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = TurnRightMoreThan180Asset;
	}
	else if (CurrRotationDirection == ERotateDirection::RIGHT && !shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = TurnRightLessThan180Asset;
	}
	else if (CurrRotationDirection == ERotateDirection::LEFT && shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = TurnLeftMoreThan180Asset;
	}
	else if (CurrRotationDirection == ERotateDirection::LEFT && !shouldDoBigRotation)
	{
		CurrPlayingTurnSequence = TurnLeftLessThan180Asset;
	}
	else
	{
		CurrPlayingTurnSequence = nullptr;
	}
}

void ABase_MyCharacter::SetRotationCurveScaleValue()
{
	float diff = FMath::Abs(CurrCharacterHorizontalAngle - CurrTargetCharacterHorizontalAngle);
	if (diff <= 90)
	{
		RotationCurveScaleValue = diff / ABase_MyCharacter::NinetyDegreeRotationCurveAmount;
	}
		
	else
	{
		RotationCurveScaleValue = diff / ABase_MyCharacter::OneHundredEightyDegreeRotationCurveAmount;
	}
}

void ABase_MyCharacter::PlayTurningMontage()
{
	SetTurnAnimationAsset();
	SetRotationCurveScaleValue();
	float playRate = 1;
	// scale play rate if moving to match movement speed
	if (CurrCharacterMovementState == ECharacterMovementState::MOVING)
	{
		playRate = FMath::Max(MovementSettings.CharacterWalkMovementSpeed / 2, CurrCharacterMovementSpeed) / MovementSettings.CharacterWalkMovementSpeed;
	}
	MainAnimInstance->PlaySlotAnimationAsDynamicMontage(CurrPlayingTurnSequence, LegsSlotName, 0, 0, playRate);
}

void ABase_MyCharacter::StopCurrentlyPlayingTurningMontage()
{
	MainAnimInstance->StopSlotAnimation(0.25, LegsSlotName);
}

void ABase_MyCharacter::UpdateCharacterRotationThroughCurve()
{
	if (MainAnimInstance)
	{
		float rotationCurveValue = FMath::Abs(MainAnimInstance->GetCurveValue(RotationCurveName) * RotationCurveScaleValue);		
				
		if (CurrRotationDirection == ERotateDirection::RIGHT)
		{
			float uncappedHorizontalAngle = CurrCharacterHorizontalAngle - rotationCurveValue;
			CurrCharacterHorizontalAngle = FMath::Max(uncappedHorizontalAngle, CurrTargetCharacterHorizontalAngle);
		}
		else if (CurrRotationDirection == ERotateDirection::LEFT)
		{
			float uncappedHorizontalAngle = CurrCharacterHorizontalAngle + rotationCurveValue;
			CurrCharacterHorizontalAngle = FMath::Min(uncappedHorizontalAngle, CurrTargetCharacterHorizontalAngle);
		}
	}
}

bool ABase_MyCharacter::IsPlayingTurningMontage() const
{
	return CurrPlayingTurnSequence && MainAnimInstance->IsPlayingSlotAnimation(CurrPlayingTurnSequence, LegsSlotName);
}
#pragma endregion

#pragma region Camera
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
	CurrViewVerticalAngle = FMath::ClampAngle(CurrViewVerticalAngle + CameraInput.Y * CameraSettings.CameraRotationalSpeed * deltaTime, CameraSettings.MinViewVerticalAngle, CameraSettings.MaxViewVerticalAngle);
	CurrViewHorizontalAngle = FMath::ClampAngle(CurrViewHorizontalAngle + CameraInput.X * CameraSettings.CameraRotationalSpeed * deltaTime, 0, 359.9);
}
#pragma endregion

#pragma region Movement and Rotation Info
FCharacterMovementSettings ABase_MyCharacter::GetMovementSettings() const
{
	return MovementSettings;
}

FCharacterState ABase_MyCharacter::GetCurrentState() const
{
	return FCharacterState{ CurrCharacterMovementState, CurrRotationDirection, CurrCharacterGait,
		CurrTargetCharacterHorizontalAngle, CurrCharacterHorizontalAngle, CurrViewVerticalAngle, CurrCharacterMovementSpeed, TargetCharacterMovementSpeed };
}
#pragma endregion

#pragma region Rope Swing
bool ABase_MyCharacter::EnterSwingState()
{
	if (CurrCharacterMovementState == ECharacterMovementState::SWINGING)
		return false;
	
	CurrCharacterMovementState = ECharacterMovementState::SWINGING;
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

	if (CurrCharacterMovementState != ECharacterMovementState::JUMPING)
	{
		if (IsPlayingTurningMontage())
		{
			UpdateCharacterRotationThroughCurve();
			SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
		}
		else
		{
			CurrTargetCharacterHorizontalAngle = NextTargetCharacterHorizontalAngle;
			CurrRotationDirection = NextRotationDirection;
			if (ShouldDoMovingRotation())
			{
				if (ShouldDoMontageRotation())
				{
					PlayTurningMontage();
				}
				else
				{
					UpdateCharacterMovingRotation(deltaTime);
					SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
				}
			}
			else if (ShouldRotateInPlace())
			{
				// TODO: Clean this up
				RotationCountdownTimer += deltaTime;
				if (RotationCountdownTimer >= MovementSettings.RotationLookTimeThreshold)
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
		CurrTargetCharacterHorizontalAngle = NextTargetCharacterHorizontalAngle;
		CurrRotationDirection = NextRotationDirection;
		UpdateCharacterMovingRotation(deltaTime);
		SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
	}

	Move(deltaTime);

	RotateCamera();

}
