// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/Base_MyCharacter.h"
#include "threecs_project/Public/MyPlayerController.h"

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

	CameraRotationalSpeed = 2;
	MaxViewVerticalAngle = 30;
	MinViewVerticalAngle = -60;

	CharacterRotationalSpeed = 1;
	CharacterMovementSpeed = 1;
}

// Called when the game starts or when spawned
void ABase_MyCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrViewVerticalAngle = 0;

	CurrViewHorizontalAngle = GetActorRotation().Yaw;
	CurrCharacterHorizontalAngle = CurrViewHorizontalAngle;
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;

	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	controller->OnCharacterMovement.AddUObject(this, &ABase_MyCharacter::OnCharacterMovement);
	controller->OnCameraMovement.AddUObject(this, &ABase_MyCharacter::OnCameraMovement);

	SetCameraRotation();
}

void ABase_MyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	if (controller)
	{
		controller->OnCharacterMovement.RemoveAll(this);
		controller->OnCameraMovement.RemoveAll(this);
	}
}

void ABase_MyCharacter::OnCharacterMovement(FVector2D movementVector)
{
	// lerp the rotation of the character towards the target horizontal angle
	// TODO: Handle small differences and large differences
	if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Max(CurrCharacterHorizontalAngle - CharacterRotationalSpeed, TargetCharacterHorizontalAngle);
	}
	else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Min(CurrCharacterHorizontalAngle + CharacterRotationalSpeed, TargetCharacterHorizontalAngle);
	}
	SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });

	AddMovementInput(FVector::ForwardVector.RotateAngleAxis(CurrCharacterHorizontalAngle, { 0, 0, 1 }), movementVector.Y * CharacterMovementSpeed);
	AddMovementInput(FVector::RightVector.RotateAngleAxis(CurrCharacterHorizontalAngle, { 0, 0, 1 }), movementVector.X * CharacterMovementSpeed);
}

void ABase_MyCharacter::OnCameraMovement(FVector2D cameraVector)
{
	CurrViewVerticalAngle = FMath::ClampAngle(CurrViewVerticalAngle + cameraVector.Y * CameraRotationalSpeed, MinViewVerticalAngle, MaxViewVerticalAngle);
	CurrViewHorizontalAngle = FMath::ClampAngle(CurrViewHorizontalAngle + cameraVector.X * CameraRotationalSpeed, 0, 359.9);
	
	// set target character horizontal angle to be from 0 - 360
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle < 0 ? 360 + CurrViewHorizontalAngle : CurrViewHorizontalAngle;
	// reverse the direction if the angle > 180
	if (FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle) > 180)
	{
		TargetCharacterHorizontalAngle = -(360 - TargetCharacterHorizontalAngle);
	}
}
