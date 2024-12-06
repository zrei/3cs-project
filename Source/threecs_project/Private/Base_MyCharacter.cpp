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
	FastCharacterRotationalSpeed = 10;
	FastRotationThreshold = 30;

	CharacterMovementSpeedChange = 2;
	CharacterWalkMovementSpeed = 4;
	CharacterRunMovementSpeed = 10;
}

// Called when the game starts or when spawned
void ABase_MyCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrCharacterMovementState = ECharacterMovementState::VE_IDLE;
	CurrCharacterGait = ECharacterGait::VE_WALK;
	TargetCharacterMovementSpeed = 0;
	CurrCharacterMovementSpeed = 0;

	HasCameraInput = false;
	CameraInput = FVector2D::Zero();
	CurrViewVerticalAngle = 0;

	CurrViewHorizontalAngle = GetActorRotation().Yaw;
	CurrCharacterHorizontalAngle = CurrViewHorizontalAngle;
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;

	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	controller->OnCharacterMovementStarted.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementStarted);
	controller->OnCharacterMovement.AddUObject(this, &ABase_MyCharacter::OnCharacterMovement);
	controller->OnCharacterMovementComplete.AddUObject(this, &ABase_MyCharacter::OnCharacterMovementComplete);
	
	controller->OnCameraMovementStarted.AddUObject(this, &ABase_MyCharacter::OnCameraMovementStarted);
	controller->OnCameraMovement.AddUObject(this, &ABase_MyCharacter::OnCameraMovement);
	controller->OnCameraMovementComplete.AddUObject(this, &ABase_MyCharacter::OnCameraMovementComplete);
	
	controller->OnGaitChange.AddUObject(this, &ABase_MyCharacter::OnGaitChange);

	SetCameraRotation();
}

void ABase_MyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	if (controller)
	{
		controller->OnCharacterMovementStarted.RemoveAll(this);
		controller->OnCharacterMovement.RemoveAll(this);
		controller->OnCharacterMovementComplete.RemoveAll(this);
		
		controller->OnCameraMovementStarted.RemoveAll(this);
		controller->OnCameraMovement.RemoveAll(this);
		controller->OnCameraMovementComplete.RemoveAll(this);
		
		controller->OnGaitChange.RemoveAll(this);
	}
}
#pragma endregion

#pragma region Movement
void ABase_MyCharacter::OnCharacterMovementStarted()
{
	CurrCharacterMovementState = ECharacterMovementState::VE_MOVING;
	// reset gait to walk
	CurrCharacterGait = ECharacterGait::VE_WALK;
	SetTargetCharacterMovementSpeed();
}

void ABase_MyCharacter::OnCharacterMovement(FVector2D movementVector)
{
	MovementInput = movementVector;
}

void ABase_MyCharacter::Move(float deltaTime)
{
	AddMovementInput(FVector::ForwardVector.RotateAngleAxis(CurrCharacterHorizontalAngle, { 0, 0, 1 }), MovementInput.Y * CurrCharacterMovementSpeed * deltaTime);
	AddMovementInput(FVector::RightVector.RotateAngleAxis(CurrCharacterHorizontalAngle, { 0, 0, 1 }), MovementInput.X * CurrCharacterMovementSpeed * deltaTime);
}

void ABase_MyCharacter::OnCharacterMovementComplete()
{
	CurrCharacterMovementState = ECharacterMovementState::VE_IDLE;
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
	if (CurrCharacterMovementState == ECharacterMovementState::VE_IDLE)
		TargetCharacterMovementSpeed = 0;
	else
		TargetCharacterMovementSpeed = CurrCharacterGait == ECharacterGait::VE_RUN ? CharacterRunMovementSpeed : CharacterWalkMovementSpeed;
}
#pragma endregion

#pragma region Gait
void ABase_MyCharacter::OnGaitChange(bool _)
{
	CurrCharacterGait = CurrCharacterGait == ECharacterGait::VE_RUN ? ECharacterGait::VE_WALK : ECharacterGait::VE_RUN;
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
	if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Max(CurrCharacterHorizontalAngle - CharacterRotationalSpeed * deltaTime, TargetCharacterHorizontalAngle);
	}
	else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Min(CurrCharacterHorizontalAngle + CharacterRotationalSpeed * deltaTime, TargetCharacterHorizontalAngle);
	}
	SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });
}
#pragma endregion

#pragma region Camera
void ABase_MyCharacter::OnCameraMovementStarted()
{
	HasCameraInput = true;
}

void ABase_MyCharacter::OnCameraMovement(FVector2D cameraVector)
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

	if (CurrCharacterMovementState == ECharacterMovementState::VE_MOVING)
	{
		SetCharacterRotation(deltaTime);
	}

	SetCharacterMovementSpeed(deltaTime);
	Move(deltaTime);

	SetCameraRotation();
}