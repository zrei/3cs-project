// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/MyCharacter_NoSpringarm.h"
#include "threecs_project/Public/MyPlayerController.h"
#include "Logging/StructuredLog.h"

// Sets default values
AMyCharacter_NoSpringarm::AMyCharacter_NoSpringarm()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraParent = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Parent"));
	CameraParent->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraParent);

	CameraRotationalSpeed = 2;
	MaxViewVerticalAngle = 30;
	MinViewVerticalAngle = -60;
	CameraLocationOffset = { -190, 0, 0 };

	CharacterRotationalSpeed = 0.5;
	CharacterMovementSpeed = 1;
}

// Called when the game starts or when spawned
void AMyCharacter_NoSpringarm::BeginPlay()
{
	Super::BeginPlay();

	CurrViewVerticalAngle = 0;

	CurrViewHorizontalAngle = GetActorRotation().Yaw;
	CurrCharacterHorizontalAngle = CurrViewHorizontalAngle;
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;

	// cast to my player controller
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	// subscribe to input events
	CharacterMovementHandle = controller->OnCharacterMovement.AddUObject(this, &AMyCharacter_NoSpringarm::OnCharacterMovement);
	CameraMovementHandle = controller->OnCameraMovement.AddUObject(this, &AMyCharacter_NoSpringarm::OnCameraMovement);
	
	// set initial camera rotation
	// rotate the camera offset vector to get the new vertical position of the camera // TODO: Perform only a single rotation?
	Camera->SetWorldLocation(CameraParent->GetComponentLocation() + CameraLocationOffset.RotateAngleAxis(CurrViewVerticalAngle, { 0, 1, 0 }).RotateAngleAxis(CurrViewHorizontalAngle, { 0, 0, 1 }));
	// rotate the camera itself so that it aligns with the vector
	Camera->SetWorldRotation(FQuat::MakeFromRotator(FRotator{ -CurrViewVerticalAngle, CurrViewHorizontalAngle, 0 }));
	
}

// Called every frame
void AMyCharacter_NoSpringarm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// have to continually update the cameera position and rotation, otherwise it will be affected by the character rotation
	// rotate the camera offset vector to get the new vertical position of the camera // TODO: Perform only a single rotation?
	Camera->SetWorldLocation(CameraParent->GetComponentLocation() + CameraLocationOffset.RotateAngleAxis(CurrViewVerticalAngle, { 0, 1, 0 }).RotateAngleAxis(CurrViewHorizontalAngle, { 0, 0, 1 }));
	// rotate the camera itself so that it aligns with the vector
	Camera->SetWorldRotation(FQuat::MakeFromRotator(FRotator{ -CurrViewVerticalAngle, CurrViewHorizontalAngle, 0 }));
}

// Called to bind functionality to input
void AMyCharacter_NoSpringarm::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyCharacter_NoSpringarm::OnCharacterMovement(FVector2D movementVector)
{
	// lerp the rotation of the character towards the target horizontal angle (TODO: Handle small differences?)
	if (CurrCharacterHorizontalAngle > TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Max(CurrCharacterHorizontalAngle - CharacterRotationalSpeed, TargetCharacterHorizontalAngle);
	}
	else if (CurrCharacterHorizontalAngle < TargetCharacterHorizontalAngle)
	{
		CurrCharacterHorizontalAngle = FMath::Min(CurrCharacterHorizontalAngle + CharacterRotationalSpeed, TargetCharacterHorizontalAngle);
	}
	SetActorRotation({ 0, CurrCharacterHorizontalAngle, 0 });

	AddMovementInput(FVector::ForwardVector.RotateAngleAxis(CurrViewHorizontalAngle, { 0, 0, 1 }), movementVector.Y * CharacterMovementSpeed);
	AddMovementInput(FVector::RightVector.RotateAngleAxis(CurrViewHorizontalAngle, { 0, 0, 1 }), movementVector.X * CharacterMovementSpeed);
}

void AMyCharacter_NoSpringarm::OnCameraMovement(FVector2D cameraVector)
{	
	CurrViewVerticalAngle = FMath::Clamp(CurrViewVerticalAngle + cameraVector.Y * CameraRotationalSpeed, MinViewVerticalAngle, MaxViewVerticalAngle);

	CurrViewHorizontalAngle += cameraVector.X * CameraRotationalSpeed;
	if (CurrViewHorizontalAngle >= 360)
	{
		CurrViewHorizontalAngle -= 360;
	}
	else if (CurrViewHorizontalAngle < 0)
	{
		CurrViewHorizontalAngle = 360 + CurrViewHorizontalAngle;
	}

	// calculate target character horizontal angle
	TargetCharacterHorizontalAngle = CurrViewHorizontalAngle;
	if (FMath::Abs(CurrCharacterHorizontalAngle - TargetCharacterHorizontalAngle) > 180)
	{
		TargetCharacterHorizontalAngle = -(360 - TargetCharacterHorizontalAngle);
	}
}

void AMyCharacter_NoSpringarm::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// cast to my player controller
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	// due to execution order the controller may no longer exist
	if (controller)
	{
		controller->OnCharacterMovement.Remove(CharacterMovementHandle);
		controller->OnCameraMovement.Remove(CameraMovementHandle);
	}
}