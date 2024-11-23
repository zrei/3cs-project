// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "MyCharacter.generated.h"

/*
Camera positioning handled by spring arm using absolute rotation to make it independent
of character rotation. View direction when moving directly uses the stored horizontal
angle and also rotates the character.

Maximum vertical camera rotation is capped.

TODO: Delta time + lerp character rotation
*/
UCLASS()
class THREECS_PROJECT_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "Camera")
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Camera")
	class USpringArmComponent* CameraSpringArm;

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

	float CurrHorizontalAngle;
};
