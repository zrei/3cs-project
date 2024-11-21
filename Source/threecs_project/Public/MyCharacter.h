// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "MyCharacter.generated.h"

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

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere)
	class USceneComponent* CameraLookPoint;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector CameraOffset;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float VerticalRotationalSpeed;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float HorizontalRotationalSpeed;

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
};
