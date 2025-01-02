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

	virtual void Tick(float deltaTime) override;

#pragma region State
private:
	ECharacterGait CurrCharacterGait;

	ECharacterMovementState CurrCharacterMovementState;

	ERotateDirection CurrRotationDirection;
#pragma endregion

#pragma region Character Movement
private:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float CharacterWalkMovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CharacterRunMovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CharacterMovementSpeedChange;

	void OnCharacterMovementTriggered(FVector2D movementVector);

	void OnCharacterMovementStarted();

	void OnCharacterMovementComplete();

	FVector2D MovementInput;

	void Move(float deltaTime);

	void SetCharacterMovementSpeed(float deltaTime);

	void SetTargetCharacterMovementSpeed();

	float GetMovementRotation() const;

	float CurrCharacterMovementSpeed;

	float TargetCharacterMovementSpeed;
#pragma endregion

#pragma region Rotation
private:
	UPROPERTY(EditAnywhere, Category = "Movement")
	float CharacterRotationalSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float FastCharacterRotationalSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float FastRotationThreshold;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float StationaryRotationThreshold;

	void SetTargetCharacterRotation();
	
	void SetCharacterRotation(float deltaTime);

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
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<USceneComponent> CameraParent;

	virtual void RotateCamera() PURE_VIRTUAL(ABase_MyCharacter::RotateCamera)

	virtual void OnCameraMovementTriggered(FVector2D cameraVector);

	float CurrViewVerticalAngle;

	float CurrViewHorizontalAngle;

private:
	UPROPERTY(EditAnywhere, Category = "Camera")
	float MinViewVerticalAngle;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxViewVerticalAngle;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraRotationalSpeed;

	void OnCameraMovementStarted();

	void OnCameraMovementComplete();

	bool HasCameraInput;

	FVector2D CameraInput;

	void SetTargetCameraRotation(float deltaTime);
#pragma endregion

#pragma region Character Info
	UPROPERTY(EditAnywhere)
	FCharacterMovementSettings MovementSettings;

	UFUNCTION(BlueprintCallable)
	bool IsRunning() const;

	UFUNCTION(BlueprintCallable)
	bool ShouldRotateInPlace() const;

	UFUNCTION(BlueprintCallable)
	ERotateDirection RotateDirection() const;

	UFUNCTION(BlueprintCallable)
	FCharacterMovementSettings GetMovementSettings() const
	{
		return MovementSettings;
	}
#pragma endregion
};
