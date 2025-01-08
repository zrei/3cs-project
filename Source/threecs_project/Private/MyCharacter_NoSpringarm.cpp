// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter_NoSpringarm.h"

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
	FVector cameraWorldLocation = CameraParent->GetComponentLocation() + CameraLocationOffset.RotateAngleAxis(-CurrViewVerticalAngle, FVector::RightVector).RotateAngleAxis(CurrViewHorizontalAngle, FVector::UpVector);
	Camera->SetWorldLocation(cameraWorldLocation);

	FQuat cameraWorldRotation = FQuat::MakeFromRotator(FRotator{ CurrViewVerticalAngle, CurrViewHorizontalAngle, 0 });
	Camera->SetWorldRotation(cameraWorldRotation);
}