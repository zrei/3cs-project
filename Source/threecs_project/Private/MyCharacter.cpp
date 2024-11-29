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

void AMyCharacter::OnCameraMovement(FVector2D cameraVector)
{	
	Super::OnCameraMovement(cameraVector);
	
	SetCameraRotation();
}

void AMyCharacter::SetCameraRotation()
{
	CameraSpringArm->SetWorldRotation(FQuat::MakeFromRotator(FRotator{ CurrViewVerticalAngle, CurrViewHorizontalAngle, 0 }));
}