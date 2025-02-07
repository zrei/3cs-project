// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Logic/MyCharacter_NoSpringarm.h"

// Sets default values
AMyCharacter_NoSpringarm::AMyCharacter_NoSpringarm()
{
	Camera->SetupAttachment(CameraParent);

	// TODO: Socket offset?
	CameraLocationOffset = { -250, 0, 0 };
}

void AMyCharacter_NoSpringarm::RotateCamera()
{
	// TODO: Perform only a single rotation?
	FVector cameraWorldLocation = CameraParent->GetComponentLocation() + CameraLocationOffset.RotateAngleAxis(-CurrCharacterState.CurrLookPitch, FVector::RightVector).RotateAngleAxis(CurrCharacterState.CurrLookYaw, FVector::UpVector);
	Camera->SetWorldLocation(cameraWorldLocation);

	FQuat cameraWorldRotation = FQuat::MakeFromRotator(FRotator{ CurrCharacterState.CurrLookPitch, CurrCharacterState.CurrLookYaw, 0 });
	Camera->SetWorldRotation(cameraWorldRotation);
}