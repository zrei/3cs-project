// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Base_MyCharacter.generated.h"

UCLASS()
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

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class UCameraComponent> Camera;

	// making it a TObjectPtr to follow Unreal's recommendation
	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<class USceneComponent> CameraParent;

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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void OnCharacterMovement(FVector2D movementVector);

	virtual void OnCameraMovement(FVector2D cameraVector);

	FDelegateHandle CharacterMovementHandle;

	FDelegateHandle CameraMovementHandle;

	float CurrCharacterHorizontalAngle;

	float TargetCharacterHorizontalAngle;

	float CurrViewVerticalAngle;

	float CurrViewHorizontalAngle;
};
