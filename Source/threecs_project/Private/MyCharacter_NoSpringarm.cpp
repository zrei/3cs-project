// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/MyCharacter_NoSpringarm.h"

// Sets default values
AMyCharacter_NoSpringarm::AMyCharacter_NoSpringarm()
{
	Camera->SetupAttachment(CameraParent);

	// TODO: Socket offset?
	CameraLocationOffset = { -250, 0, 0 };
}

void AMyCharacter_NoSpringarm::SetCameraRotation()
{
	// TODO: Perform only a single rotation?
	Camera->SetWorldLocation(CameraParent->GetComponentLocation() + CameraLocationOffset.RotateAngleAxis(-CurrViewVerticalAngle, { 0, 1, 0 }).RotateAngleAxis(CurrViewHorizontalAngle, { 0, 0, 1 }));
	Camera->SetWorldRotation(FQuat::MakeFromRotator(FRotator{ CurrViewVerticalAngle, CurrViewHorizontalAngle, 0 }));
}