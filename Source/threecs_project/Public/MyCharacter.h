// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class USpringArmComponent;

#include "CoreMinimal.h"
#include "threecs_project/Public/Base_MyCharacter.h"
#include "MyCharacter.generated.h"

/*
Camera positioning handled by spring arm using absolute rotation to make it independent
of character rotation. 
*/
UCLASS()
class THREECS_PROJECT_API AMyCharacter : public ABase_MyCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	virtual void OnCameraMovement(FVector2D cameraVector) override;

	virtual void SetCameraRotation() override;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraSpringArm;
};
