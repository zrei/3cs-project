// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Base_MyCharacter.generated.h"

/*
View direction is calculated and stored whenever the camera is adjusted.
Movement moves in the character's current facing direction while lerping
it towards the view direction.

Vertical view direction is capped with a minimum and maximum angle.
*/
UCLASS(Abstract)
class THREECS_PROJECT_API ABase_MyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABase_MyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetCameraRotation();

	virtual void OnCameraMovement(FVector2D cameraVector);

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;

	// making it a TObjectPtr to follow Unreal's recommendation
	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class USceneComponent> CameraParent;

	float CurrViewVerticalAngle;

	float CurrViewHorizontalAngle;

private:
	void OnCharacterMovement(FVector2D movementVector);

	FDelegateHandle CharacterMovementHandle;

	FDelegateHandle CameraMovementHandle;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MinViewVerticalAngle;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxViewVerticalAngle;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraRotationalSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CharacterMovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CharacterRotationalSpeed;

	float CurrCharacterHorizontalAngle;

	float TargetCharacterHorizontalAngle;
};
