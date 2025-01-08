// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UCameraComponent;
class USceneComponent;

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "CharacterState.h"
#include "Base_MyCharacter.generated.h"

/*
View direction is calculated and stored whenever the camera is adjusted.

When moving, the forward direction will follow the camera view, with movement
in other directions changing the target character rotation. Character rotation
is always lerped.

When stationary, character will turn to follow the camera view.

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

	virtual void Tick(float deltaTime) override;

#pragma region State
private:
	ECharacterGait CurrCharacterGait;

	ECharacterMovementState CurrCharacterMovementState;

	ERotateDirection CurrRotationDirection;
#pragma endregion

#pragma region Character Movement
private:
	void OnCharacterMovementTriggered(FVector2D movementVector);

	void OnCharacterMovementStarted();

	void OnCharacterMovementComplete();

	void Move(float deltaTime);

	void SetCharacterMovementSpeed(float deltaTime);

	void SetTargetCharacterMovementSpeed();

	float GetMovementRotation() const;

	float CurrCharacterMovementSpeed;

	float TargetCharacterMovementSpeed;

	FVector2D MovementInput;
#pragma endregion

#pragma region Rotation
public:
	UFUNCTION(BlueprintCallable)
	bool ShouldRotateInPlace() const;

private:
	bool ShouldDoMovingRotation() const;

	void SetTargetCharacterRotation();
	
	void UpdateCharacterMovingRotation(float deltaTime);

	void UpdateCharacterGroundedRotation(float deltaTime);

	float CurrCharacterHorizontalAngle;

	float TargetCharacterHorizontalAngle;
#pragma endregion

#pragma region Gait
private:
	void OnGaitChangeTriggered();
#pragma endregion

#pragma region Camera
protected:
	UPROPERTY(EditAnywhere, Category = "Camera")
	FCameraSettings CameraSettings;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<USceneComponent> CameraParent;

	virtual void RotateCamera() PURE_VIRTUAL(ABase_MyCharacter::RotateCamera)

	virtual void OnCameraMovementTriggered(FVector2D cameraVector);

	float CurrViewVerticalAngle;

	float CurrViewHorizontalAngle;

private:
	void OnCameraMovementStarted();

	void OnCameraMovementComplete();

	bool HasCameraInput;

	FVector2D CameraInput;

	void SetTargetCameraRotation(float deltaTime);
#pragma endregion

#pragma region Movement and Rotation Info
protected:
	UPROPERTY(EditAnywhere, Category = "Movement")
	FCharacterMovementSettings MovementSettings;

public:
	UFUNCTION(BlueprintCallable)
	FCharacterMovementSettings GetMovementSettings() const;

	UFUNCTION(BlueprintCallable)
	FCharacterState GetCurrentState() const;
#pragma endregion

#pragma region Skeleton and Animation
protected:
	UPROPERTY(EditAnywhere, Category = "Animation")
	FName RotationCurveName;

private:
	TObjectPtr<UAnimInstance> MainAnimInstance;
#pragma endregion
};
