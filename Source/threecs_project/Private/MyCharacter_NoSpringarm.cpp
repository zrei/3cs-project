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

	RotationalSpeed = 5;
	MovementSpeed = 1;
	MaxUpwardsAngle = 20;
	MinDownwardsAngle = -30;

	CameraOffset = { 0, -190, 0 };
}

// Called when the game starts or when spawned
void AMyCharacter_NoSpringarm::BeginPlay()
{
	Super::BeginPlay();

	CurrVerticalAngle = 0;

	// cast to my player controller
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	CharacterMovementHandle = controller->OnCharacterMovement.AddUObject(this, &AMyCharacter_NoSpringarm::OnCharacterMovement);
	CameraMovementHandle = controller->OnCameraMovement.AddUObject(this, &AMyCharacter_NoSpringarm::OnCameraMovement);
	
	// offset the camera to its original offset position
	Camera->SetRelativeLocation(CameraOffset, false);
}

// Called every frame
void AMyCharacter_NoSpringarm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCharacter_NoSpringarm::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyCharacter_NoSpringarm::OnCharacterMovement(FVector2D movementVector)
{
	AddMovementInput(CameraParent->GetForwardVector(), movementVector.Y * MovementSpeed);
	AddMovementInput(CameraParent->GetRightVector(), movementVector.X * MovementSpeed);
	UE_LOG(LogTemp, Warning, TEXT("Character move"));
	UE_LOGFMT(LogTemp, Warning, "Movement Vector: {0}, {1}", movementVector.X, movementVector.Y);
}

void AMyCharacter_NoSpringarm::OnCameraMovement(FVector2D cameraVector)
{
	UE_LOGFMT(LogTemp, Warning, "Camera Vector: {0}, {1}", cameraVector.X, cameraVector.Y);
	
	CurrVerticalAngle = FMath::Clamp(CurrVerticalAngle + cameraVector.Y * RotationalSpeed, MinDownwardsAngle, MaxUpwardsAngle);
	UE_LOGFMT(LogTemp, Warning, "Vertical angle: {0}", CurrVerticalAngle);
	
	// Possible: Use absolutes instead
	// rotate the camera offset vector to get the new vertical position of the camera
	Camera->SetRelativeLocation(CameraOffset.RotateAngleAxis(CurrVerticalAngle, { 0, 1, 0 }));
	// rotate the camera itself so that it aligns with the vector
	Camera->SetRelativeRotation(FQuat::MakeFromRotator(FRotator{ -CurrVerticalAngle, 0, 0 }));

	
	CameraParent->AddLocalRotation(FQuat::MakeFromRotator(FRotator{ 0, cameraVector.X * RotationalSpeed, 0 }));
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