// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "threecs_project/Public/Base_MyCharacter.h"
#include "MyCharacter_NoSpringarm.generated.h"

/*
Camera positioning handled internally - absolute rotation and location
are set every tick to make it independent of character rotation.
No collision handling at the moment so the camera passes through things.

Possible to move the entire camera to the camera manager.
*/
UCLASS()
class THREECS_PROJECT_API AMyCharacter_NoSpringarm : public ABase_MyCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter_NoSpringarm();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SetCameraRotation() override;

	// TODO: Socket offset
	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector CameraLocationOffset;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
