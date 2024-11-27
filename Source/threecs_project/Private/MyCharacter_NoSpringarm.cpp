// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/MyCharacter_NoSpringarm.h"

// Sets default values
AMyCharacter_NoSpringarm::AMyCharacter_NoSpringarm()
{
	Camera->SetupAttachment(CameraParent);

	// TODO: Socket offset?
	CameraLocationOffset = { -250, 0, 0 };
}

// Called when the game starts or when spawned
void AMyCharacter_NoSpringarm::BeginPlay()
{
	Super::BeginPlay();

	// set initial camera rotation
	SetCameraRotation();
}

// Called every frame
void AMyCharacter_NoSpringarm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// have to continually update the cameera position and rotation, otherwise it will be affected by the character rotation
	SetCameraRotation();
}

void AMyCharacter_NoSpringarm::SetCameraRotation()
{
	// rotate the camera offset vector to get the new vertical position of the camera // TODO: Perform only a single rotation?
	Camera->SetWorldLocation(CameraParent->GetComponentLocation() + CameraLocationOffset.RotateAngleAxis(CurrViewVerticalAngle, { 0, 1, 0 }).RotateAngleAxis(CurrViewHorizontalAngle, { 0, 0, 1 }));
	// rotate the camera itself so that it aligns with the vector
	Camera->SetWorldRotation(FQuat::MakeFromRotator(FRotator{ -CurrViewVerticalAngle, CurrViewHorizontalAngle, 0 }));
}