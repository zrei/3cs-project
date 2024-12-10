// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/Base_MyCharacter.h"
#include "threecs_project/Public/MyPlayerController.h"

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

	CameraRotationalSpeed = 10;
	MaxViewVerticalAngle = 30;
	MinViewVerticalAngle = -60;

	CharacterRotationalSpeed = 4;
	FastCharacterRotationalSpeed = 30;
	FastRotationThreshold = 30;

	CharacterMovementSpeedChange = 0.15;
	CharacterWalkMovementSpeed = 0.3;
	CharacterRunMovementSpeed = 0.7;
}

// Called when the game starts or when spawned
void ABase_MyCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrCharacterMovementState = ECharacterMovementState::IDLE;
	CurrCharacterGait = ECharacterGait::WALK;
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
}

void ABase_MyCharacter::Move(float deltaTime)
{
	FVector forwardDirection = FVector::ForwardVector.RotateAngleAxis(CurrCharacterHorizontalAngle, FVector::UpVector);
	float forwardMovementAmount = MovementInput.Y * CurrCharacterMovementSpeed;
	AddMovementInput(forwardDirection, forwardMovementAmount);

	FVector rightDirection = FVector::RightVector.RotateAngleAxis(CurrCharacterHorizontalAngle, FVector::UpVector);
	float rightMovementAmount = MovementInput.X * CurrCharacterMovementSpeed;
	AddMovementInput(rightDirection, rightMovementAmount);
}

void ABase_MyCharacter::OnCharacterMovementComplete()
{
	CurrCharacterMovementState = ECharacterMovementState::IDLE;
	SetTargetCharacterMovementSpeed();
}

void ABase_MyCharacter::SetCharacterMovementSpeed(float deltaTime)
{
	if (TargetCharacterMovementSpeed > CurrCharacterMovementSpeed)
	{
		CurrCharacterMovementSpeed = FMath::Min(CurrCharacterMovementSpeed + CharacterMovementSpeedChange * deltaTime, TargetCharacterMovementSpeed);
	}
	else if (TargetCharacterMovementSpeed < CurrCharacterMovementSpeed)
	{
		CurrCharacterMovementSpeed = FMath::Max(CurrCharacterMovementSpeed - CharacterMovementSpeedChange * deltaTime, TargetCharacterMovementSpeed);
	}
}

void ABase_MyCharacter::SetTargetCharacterMovementSpeed()
{
	if (CurrCharacterMovementState == ECharacterMovementState::IDLE)
		TargetCharacterMovementSpeed = 0;
	else
		TargetCharacterMovementSpeed = CurrCharacterGait == ECharacterGait::RUN ? CharacterRunMovementSpeed : CharacterWalkMovementSpeed;
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
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle < 0 ? 360 + CurrViewHorizontalAngle : CurrViewHorizontalAngle;
	// reverse the direction if the angle > 180
	if (FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle) > 180)
	{
		TargetCharacterHorizontalAngle = -(360 - TargetCharacterHorizontalAngle);
	}
}

void ABase_MyCharacter::SetCharacterRotation(float deltaTime)
{
	// lerp the rotation of the character towards the target horizontal angle
	// TODO: Handle small differences and large differences
	float rotationSpeed = FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle) > FastRotationThreshold ? FastCharacterRotationalSpeed : CharacterRotationalSpeed;
	if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Max(CurrCharacterHorizontalAngle - rotationSpeed * deltaTime, TargetCharacterHorizontalAngle);
	}
	else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Min(CurrCharacterHorizontalAngle + rotationSpeed * deltaTime, TargetCharacterHorizontalAngle);
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
	CurrViewVerticalAngle = FMath::ClampAngle(CurrViewVerticalAngle + CameraInput.Y * CameraRotationalSpeed * deltaTime, MinViewVerticalAngle, MaxViewVerticalAngle);
	CurrViewHorizontalAngle = FMath::ClampAngle(CurrViewHorizontalAngle + CameraInput.X * CameraRotationalSpeed * deltaTime, 0, 359.9);
}
#pragma endregion

void ABase_MyCharacter::Tick(float deltaTime)
{
	if (HasCameraInput)
	{
		SetTargetCameraRotation(deltaTime);
		SetTargetCharacterRotation();
	}

	if (CurrCharacterMovementState == ECharacterMovementState::MOVING)
	{
		SetCharacterRotation(deltaTime);
		SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
	}

	SetCharacterMovementSpeed(deltaTime);
	Move(deltaTime);

	RotateCamera();
}