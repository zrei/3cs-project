// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/Base_MyCharacter.h"
#include "threecs_project/Public/MyPlayerController.h"
#include "Math/UnrealMathUtility.h"

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

	MovementSettings.CharacterMovementSpeedChange = 0.15;
	MovementSettings.CharacterRotationalSpeed = 15;
	MovementSettings.CharacterRunMovementSpeed = 0.7;
	MovementSettings.CharacterWalkMovementSpeed = 0.3;
	MovementSettings.FastCharacterRotationalSpeed = 70;
	MovementSettings.StationaryRotationThreshold = 5;

	CameraSettings.CameraRotationalSpeed = 10;
	CameraSettings.MaxViewVerticalAngle = 30;
	CameraSettings.MinViewVerticalAngle = -60;

	RotationCurveName = FName{ "RotationAmount" };
}

// Called when the game starts or when spawned
void ABase_MyCharacter::BeginPlay()
{
	Super::BeginPlay();

	SkeletalMesh = FindComponentByClass<USkeletalMeshComponent>();
	if (SkeletalMesh)
		MainAnimInstance = SkeletalMesh->GetAnimInstance();

	CurrCharacterMovementState = ECharacterMovementState::IDLE;
	CurrCharacterGait = ECharacterGait::WALK;
	CurrRotationDirection = ERotateDirection::NONE;
	TargetCharacterMovementSpeed = 0;
	CurrCharacterMovementSpeed = 0;

	HasCameraInput = false;
	CameraInput = FVector2D::Zero();
	CurrViewVerticalAngle = 0;

	CurrViewHorizontalAngle = GetActorRotation().Yaw;
	CurrCharacterHorizontalAngle = CurrViewHorizontalAngle;
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;

	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	controller->OnCharacterMovementInputStarted.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementStarted);
	controller->OnCharacterMovementInputTriggered.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementTriggered);
	controller->OnCharacterMovementInputComplete.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementComplete);
	
	controller->OnCameraMovementInputStarted.AddUObject(this, &ABase_MyCharacter::OnCameraMovementStarted);
	controller->OnCameraMovementInputTriggered.AddUObject(this, &ABase_MyCharacter::OnCameraMovementTriggered);
	controller->OnCameraMovementInputComplete.AddUObject(this, &ABase_MyCharacter::OnCameraMovementComplete);
	
	controller->OnGaitChangeInputStarted.AddUObject(this, &ABase_MyCharacter::OnGaitChangeTriggered);

	RotateCamera();
}

void ABase_MyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	if (controller)
	{
		controller->OnCharacterMovementInputStarted.RemoveAll(this);
		controller->OnCharacterMovementInputTriggered.RemoveAll(this);
		controller->OnCharacterMovementInputComplete.RemoveAll(this);
		
		controller->OnCameraMovementInputStarted.RemoveAll(this);
		controller->OnCameraMovementInputTriggered.RemoveAll(this);
		controller->OnCameraMovementInputComplete.RemoveAll(this);
		
		controller->OnGaitChangeInputStarted.RemoveAll(this);
	}
}
#pragma endregion

#pragma region Movement
void ABase_MyCharacter::OnCharacterMovementStarted()
{
	CurrCharacterMovementState = ECharacterMovementState::MOVING;
	// reset gait to walk
	CurrCharacterGait = ECharacterGait::WALK;
	SetTargetCharacterMovementSpeed();
}

void ABase_MyCharacter::OnCharacterMovementTriggered(FVector2D movementVector)
{
	MovementInput = movementVector;
	SetTargetCharacterRotation();
}

void ABase_MyCharacter::Move(float deltaTime)
{
	FVector forwardDirection = GetActorForwardVector(); //FVector::ForwardVector.RotateAngleAxis(CurrCharacterHorizontalAngle, FVector::UpVector);
	float forwardMovementAmount = MovementInput.Y * CurrCharacterMovementSpeed;
	AddMovementInput(forwardDirection, forwardMovementAmount);

	FVector rightDirection = GetActorRightVector(); //FVector::RightVector.RotateAngleAxis(CurrCharacterHorizontalAngle, FVector::UpVector);
	float rightMovementAmount = MovementInput.X * CurrCharacterMovementSpeed;
	AddMovementInput(rightDirection, rightMovementAmount);
}

void ABase_MyCharacter::OnCharacterMovementComplete()
{
	CurrCharacterMovementState = ECharacterMovementState::IDLE;
	SetTargetCharacterMovementSpeed();
	SetTargetCharacterRotation();
}

void ABase_MyCharacter::SetCharacterMovementSpeed(float deltaTime)
{
	if (TargetCharacterMovementSpeed > CurrCharacterMovementSpeed)
	{
		CurrCharacterMovementSpeed = FMath::Min(CurrCharacterMovementSpeed + MovementSettings.CharacterMovementSpeedChange * deltaTime, TargetCharacterMovementSpeed);
	}
	else if (TargetCharacterMovementSpeed < CurrCharacterMovementSpeed)
	{
		CurrCharacterMovementSpeed = FMath::Max(CurrCharacterMovementSpeed - MovementSettings.CharacterMovementSpeedChange * deltaTime, TargetCharacterMovementSpeed);
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
	float movementRotation = FMath::Acos(FVector2D{0, 1}.Dot(MovementInput)) * (180 / PI);
	if (MovementInput.X < 0)
		movementRotation = -movementRotation;
	return movementRotation;
}
#pragma endregion

#pragma region Gait
void ABase_MyCharacter::OnGaitChangeTriggered()
{
	CurrCharacterGait = CurrCharacterGait == ECharacterGait::RUN ? ECharacterGait::WALK : ECharacterGait::RUN;
	SetTargetCharacterMovementSpeed();
}
#pragma endregion

#pragma region Rotation
void ABase_MyCharacter::SetTargetCharacterRotation()
{
	// set target character horizontal angle to be from 0 - 360
	if (CurrCharacterMovementState == ECharacterMovementState::MOVING)
	{
		float rotationFromMovementInput = GetMovementRotation();
		TargetCharacterHorizontalAngle = CurrViewHorizontalAngle + rotationFromMovementInput;
	}
	else
	{
		TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;
	}
	TargetCharacterHorizontalAngle = TargetCharacterHorizontalAngle < 0 ? 360 + TargetCharacterHorizontalAngle : TargetCharacterHorizontalAngle;
	// reverse the direction if the angle > 180
	if (FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle) > 180)
	{
		TargetCharacterHorizontalAngle = -(360 - TargetCharacterHorizontalAngle);
	}

	if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
	{
		CurrRotationDirection = ERotateDirection::LEFT;
	}
	else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
	{
		CurrRotationDirection = ERotateDirection::RIGHT;
	}
	else
	{
		CurrRotationDirection = ERotateDirection::NONE;
	}
}

void ABase_MyCharacter::SetCharacterRotation(float deltaTime)
{
	// lerp the rotation of the character towards the target horizontal angle
	// TODO: Handle small differences and large differences
	if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Max(CurrCharacterHorizontalAngle - MovementSettings.FastCharacterRotationalSpeed * deltaTime, TargetCharacterHorizontalAngle);
		
	}
	else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Min(CurrCharacterHorizontalAngle + MovementSettings.FastCharacterRotationalSpeed * deltaTime, TargetCharacterHorizontalAngle);
	}
}

bool ABase_MyCharacter::ShouldRotateInPlace() const
{
	return CurrCharacterMovementState == ECharacterMovementState::IDLE && FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle) > MovementSettings.StationaryRotationThreshold;
}

ERotateDirection ABase_MyCharacter::RotateDirection() const
{
	return CurrRotationDirection;
}

void ABase_MyCharacter::UpdateCharacterGroundedRotation(float deltaTime)
{
	if (MainAnimInstance)
	{
		float rotationCurveValue = MainAnimInstance->GetCurveValue(RotationCurveName);
		if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
		{
			CurrCharacterHorizontalAngle = FMath::Max(CurrCharacterHorizontalAngle + rotationCurveValue * MovementSettings.CharacterRotationalSpeed * deltaTime, TargetCharacterHorizontalAngle);
		}
		else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
		{
			CurrCharacterHorizontalAngle = FMath::Min(CurrCharacterHorizontalAngle + rotationCurveValue * MovementSettings.CharacterRotationalSpeed * deltaTime, TargetCharacterHorizontalAngle);
		}
	}
}

#pragma endregion

#pragma region Camera
void ABase_MyCharacter::OnCameraMovementStarted()
{
	HasCameraInput = true;
}

void ABase_MyCharacter::OnCameraMovementTriggered(FVector2D cameraVector)
{
	CameraInput = cameraVector;
}

void ABase_MyCharacter::OnCameraMovementComplete()
{
	HasCameraInput = false;
}

void ABase_MyCharacter::SetTargetCameraRotation(float deltaTime)
{
	CurrViewVerticalAngle = FMath::ClampAngle(CurrViewVerticalAngle + CameraInput.Y * CameraSettings.CameraRotationalSpeed * deltaTime, CameraSettings.MinViewVerticalAngle, CameraSettings.MaxViewVerticalAngle);
	CurrViewHorizontalAngle = FMath::ClampAngle(CurrViewHorizontalAngle + CameraInput.X * CameraSettings.CameraRotationalSpeed * deltaTime, 0, 359.9);
}
#pragma endregion

void ABase_MyCharacter::Tick(float deltaTime)
{
	if (HasCameraInput)
	{
		SetTargetCameraRotation(deltaTime);
		SetTargetCharacterRotation();
	}

	if (CurrCharacterMovementState == ECharacterMovementState::MOVING && TargetCharacterHorizontalAngle != CurrCharacterHorizontalAngle)
	{
		SetCharacterRotation(deltaTime);
		SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
	}
	else if (ShouldRotateInPlace())
	{
		UpdateCharacterGroundedRotation(deltaTime);
		SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
	}

	SetCharacterMovementSpeed(deltaTime);
	Move(deltaTime);

	RotateCamera();
}

#pragma region Character Information
bool ABase_MyCharacter::IsRunning() const
{
	return CurrCharacterMovementState == ECharacterMovementState::MOVING && CurrCharacterGait == ECharacterGait::RUN;
}

FCharacterMovementSettings ABase_MyCharacter::GetMovementSettings() const
{
	return MovementSettings;
}

FCharacterState ABase_MyCharacter::GetCurrentState() const
{
	return FCharacterState{CurrCharacterMovementState, CurrRotationDirection, CurrCharacterGait, 
		TargetCharacterHorizontalAngle, CurrCharacterHorizontalAngle, CurrCharacterMovementSpeed, TargetCharacterMovementSpeed};
}
#pragma endregion