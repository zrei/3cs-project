// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Spring Arm"));
	CameraSpringArm->SetupAttachment(CameraParent);
	CameraSpringArm->SetUsingAbsoluteRotation(true);
	
	Camera->SetupAttachment(CameraSpringArm);
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// set initial camera rotation
	SetCameraRotation();
}

void AMyCharacter::OnCameraMovement(FVector2D cameraVector)
{	
	Super::OnCameraMovement(cameraVector);
	
	SetCameraRotation();
}

void AMyCharacter::SetCameraRotation()
{
	// rotate the spring arm in world space according to the vertical and horizontal view angle
	CameraSpringArm->SetWorldRotation(FQuat::MakeFromRotator(FRotator{ CurrViewVerticalAngle, CurrViewHorizontalAngle, 0 }));
}