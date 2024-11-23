// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "MyCharacter_NoSpringarm.generated.h"

/*
Camera positioning handled internally, still affected by pawn rotation atm.
Horizontal camera rotation is handled by rotating the overall
camera parent, vertical camera rotation by rotating the initial offset vector
and setting the distance + rotating the camera itself.

Possible to move the entire camera to the camera manager.

Maximum vertical camera rotation is capped.

TODO: Delta time, make rotation absolute, set character rotation
*/
UCLASS()
class THREECS_PROJECT_API AMyCharacter_NoSpringarm : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter_NoSpringarm();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "Camera")
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Camera")
	class USceneComponent* CameraParent;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MinDownwardsAngle;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxUpwardsAngle;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float RotationalSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector CameraOffset;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void OnCharacterMovement(FVector2D movementVector);

	void OnCameraMovement(FVector2D cameraVector);

	FDelegateHandle CharacterMovementHandle;

	FDelegateHandle CameraMovementHandle;

	float CurrVerticalAngle;
};
