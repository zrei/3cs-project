// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "MyCharacter.generated.h"

/*
Camera positioning handled by spring arm, might need to turn off the rotation
inheritance later. Horizontal camera rotation is handled by rotating the overall
camera parent, vertical camera rotation by rotating the springarm.

Maximum vertical camera rotation is capped.

Still need to handle delta time?
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
};
