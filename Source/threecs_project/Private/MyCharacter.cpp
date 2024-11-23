// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/MyCharacter.h"
#include "threecs_project/Public/MyPlayerController.h"
#include "Logging/StructuredLog.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraParent = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Parent"));
	CameraParent->SetupAttachment(Cast<USceneComponent>(GetCapsuleComponent()));
	
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Spring Arm"));
	CameraSpringArm->SetupAttachment(CameraParent);
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraSpringArm);

	RotationalSpeed = 5;
	MovementSpeed = 1;
	MaxUpwardsAngle = 20;
	MinDownwardsAngle = -30;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrVerticalAngle = 0;
	CurrHorizontalAngle = 0;
	
	// cast to my player controller
	AMyPlayerController* controller = Cast<AMyPlayerController>(GetController());

	CharacterMovementHandle = controller->OnCharacterMovement.AddUObject(this, &AMyCharacter::OnCharacterMovement);
	CameraMovementHandle = controller->OnCameraMovement.AddUObject(this, &AMyCharacter::OnCameraMovement);
}

void AMyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
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
	SetActorRotation({0, CurrHorizontalAngle, 0});
	AddMovementInput(FVector::ForwardVector.RotateAngleAxis(CurrHorizontalAngle, { 0, 0, 1 }), movementVector.Y * MovementSpeed);
	AddMovementInput(FVector::RightVector.RotateAngleAxis(CurrHorizontalAngle, { 0, 0, 1 }), movementVector.X * MovementSpeed);
	UE_LOGFMT(LogTemp, Warning, "Movement Vector: {0}, {1}", movementVector.X, movementVector.Y);
}

void AMyCharacter::OnCameraMovement(FVector2D cameraVector)
{
	UE_LOGFMT(LogTemp, Warning, "Camera Vector: {0}, {1}", cameraVector.X, cameraVector.Y);
	
	// recalculate vertical angle
	CurrVerticalAngle = FMath::Clamp(CurrVerticalAngle + cameraVector.Y * RotationalSpeed, MinDownwardsAngle, MaxUpwardsAngle);
	UE_LOGFMT(LogTemp, Warning, "vertical angle: {0}", CurrVerticalAngle);

	CurrHorizontalAngle += cameraVector.X * RotationalSpeed;
	if (CurrHorizontalAngle > 360)
	{
		CurrHorizontalAngle -= 360;
	}
	else if (CurrHorizontalAngle < 0)
	{
		CurrHorizontalAngle = 360 - CurrHorizontalAngle;
	}
	
	// rotate the spring arm relative to the parent
	CameraSpringArm->SetWorldRotation(FQuat::MakeFromRotator(FRotator{ CurrVerticalAngle, CurrHorizontalAngle, 0 }));
	AddControllerYawInput(cameraVector.X * RotationalSpeed);
	//CameraSpringArm->AddWorldRotation(FQuat::MakeFromRotator(FRotator{ 0, cameraVector.X/* * RotationalSpeed*/, 0 }));
	// horizontally rotate the camera parent to rotate the entire setup
	// can be replaced by rotating the spring arm relative to the z axis, or the world-z axis
	//CameraParent->AddLocalRotation(FQuat::MakeFromRotator(FRotator{ 0, cameraVector.X * RotationalSpeed, 0 }));
}

