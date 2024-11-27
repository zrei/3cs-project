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

	// set initial view angle based on initial actor rotation
	CurrViewHorizontalAngle = GetActorRotation().Yaw;
	CurrCharacterHorizontalAngle = CurrViewHorizontalAngle;
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;

	// cast to my player controller
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	// subscribe to input events
	CharacterMovementHandle = controller->OnCharacterMovement.AddUObject(this, &ABase_MyCharacter::OnCharacterMovement);
	CameraMovementHandle = controller->OnCameraMovement.AddUObject(this, &ABase_MyCharacter::OnCameraMovement);
}

void ABase_MyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// cast to my player controller
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	// due to execution order the controller may no longer exist
	if (controller)
	{
		// unsubscribe from input events
		controller->OnCharacterMovement.Remove(CharacterMovementHandle);
		controller->OnCameraMovement.Remove(CameraMovementHandle);
	}
}

void ABase_MyCharacter::OnCharacterMovement(FVector2D movementVector)
{
	// lerp the rotation of the character towards the target horizontal angle
	// Can also be achieved by either setting the control rotation of the controller and setting
	// bUseControllerDesiredRotation to true
	// OR setting bOrientRotationToMovement to true
	// TODO: Handle small differences?
	if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Max(CurrCharacterHorizontalAngle - CharacterRotationalSpeed, TargetCharacterHorizontalAngle);
	}
	else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Min(CurrCharacterHorizontalAngle + CharacterRotationalSpeed, TargetCharacterHorizontalAngle);
	}
	SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });

	// perform movement in the character's view direction
	AddMovementInput(FVector::ForwardVector.RotateAngleAxis(CurrCharacterHorizontalAngle, { 0, 0, 1 }), movementVector.Y * CharacterMovementSpeed);
	AddMovementInput(FVector::RightVector.RotateAngleAxis(CurrCharacterHorizontalAngle, { 0, 0, 1 }), movementVector.X * CharacterMovementSpeed);
}

void ABase_MyCharacter::OnCameraMovement(FVector2D cameraVector)
{
	// recalculate view vertical angle
	CurrViewVerticalAngle = FMath::Clamp(CurrViewVerticalAngle + cameraVector.Y * CameraRotationalSpeed, MinViewVerticalAngle, MaxViewVerticalAngle);

	// recalculate view horizontal angle
	CurrViewHorizontalAngle += cameraVector.X * CameraRotationalSpeed;
	if (CurrViewHorizontalAngle >= 360)
	{
		CurrViewHorizontalAngle -= 360;
	}
	else if (CurrViewHorizontalAngle < 0)
	{
		CurrViewHorizontalAngle = 360 + CurrViewHorizontalAngle;
	}

	// calculate target character horizontal angle, setting the rotation direction (clockwise
	// or anti-clockwise) based on the angle
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;
	if (FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle) > 180)
	{
		TargetCharacterHorizontalAngle = -(360 - TargetCharacterHorizontalAngle);
	}
}

void ABase_MyCharacter::SetCameraRotation()
{
	// Can't make an abstract function in a UCLASS, so putting this log here
	UE_LOG(LogScript, Fatal, TEXT("This shouldn't be called"));
}