// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Logic/MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Spring Arm"));
	CameraSpringArm->SetupAttachment(CameraParent);
	CameraSpringArm->SetUsingAbsoluteRotation(true);
	
	Camera->SetupAttachment(CameraSpringArm);
}

void AMyCharacter::RotateCamera()
{
	FQuat springarmWorldRotation = FQuat::MakeFromRotator(CurrCharacterState.CurrCameraRotation);
	CameraSpringArm->SetWorldRotation(springarmWorldRotation);
}