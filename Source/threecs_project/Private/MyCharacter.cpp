// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/MyCharacter.h"
#include "threecs_project/Public/MyPlayerController.h"
#include "Logging/StructuredLog.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraLookPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CameraLookPoint"));
	CameraLookPoint->SetupAttachment(RootComponent);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraLookPoint);

	VerticalRotationalSpeed = 5;
	HorizontalRotationalSpeed = 5;
	MovementSpeed = 1;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// cast to my player controller
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	CharacterMovementHandle = controller->OnCharacterMovement.AddUObject(this, &AMyCharacter::OnCharacterMovement);
	CameraMovementHandle = controller->OnCameraMovement.AddUObject(this, &AMyCharacter::OnCameraMovement);

	// offset the camera to its original offsetted position
	Camera->SetRelativeLocation(CameraOffset, false);
}

void AMyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// cast to my player controller
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());
	if (controller)
	{
		controller->OnCharacterMovement.Remove(CharacterMovementHandle);
		controller->OnCameraMovement.Remove(CameraMovementHandle);
	}
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyCharacter::OnCharacterMovement(FVector2D movementVector)
{
	AddMovementInput(GetActorForwardVector(), movementVector.Y * MovementSpeed);
	AddMovementInput(GetActorRightVector(), movementVector.X * MovementSpeed);
	UE_LOG(LogTemp, Warning, TEXT("Character move"));
	UE_LOGFMT(LogTemp, Warning, "Movement Vector: {0}, {1}", movementVector.X, movementVector.Y);
}

void AMyCharacter::OnCameraMovement(FVector2D cameraVector)
{
	//Camera->AddLocalRotation(FQuat::MakeFromRotator(FRotator{ cameraVector.Y * VerticalRotationalSpeed, 0, 0}));
	//FQuat xRotation { 0, 0, cameraVector.X * 20, 0 };
	//Camera->AddLocalOffset(xRotation.RotateVector(FVector(0, 1, 0)));
	Camera->AddLocalOffset(CameraOffset.RotateAngleAxis(cameraVector.X * HorizontalRotationalSpeed, FVector{ 0, 0, 1 }) - CameraOffset);
	UE_LOG(LogTemp, Warning, TEXT("Camera"));
	UE_LOGFMT(LogTemp, Warning, "Camera Vector: {0}, {1}", cameraVector.X, cameraVector.Y);
}

