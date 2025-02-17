// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UCameraComponent;
class USceneComponent;
struct FInputActionInstance;

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Character/Logic/CharacterState.h"
#include "Base_MyCharacter.generated.h"

/*
View direction is calculated and stored whenever the camera is adjusted.

When moving, the forward direction will follow the camera view, with movement
in other directions changing the target character rotation. Character rotation
is mostly driven by a turn animation montage.

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

	ERotateDirection NextRotationDirection;
#pragma endregion

#pragma region Character Movement
public:
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

private:
	void OnCharacterMovementTriggered(const FInputActionInstance& inputActionInstance);

	void OnCharacterMovementStarted(const FInputActionInstance& inputActionInstance);

	void OnCharacterMovementComplete(const FInputActionInstance& inputActionInstance);

	void Move(float deltaTime);

	void SetCharacterMovementSpeed(float deltaTime);

	void SetTargetCharacterMovementSpeed();

	float GetMovementRotation() const;

	void OnCharacterJump(const FInputActionInstance& inputActionInstance);

	float CurrCharacterMovementSpeed;

	float TargetCharacterMovementSpeed;

	FVector2D MovementInput;
#pragma endregion

#pragma region Rotation
protected:
	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	TObjectPtr<UAnimSequenceBase> TurnLeftLessThan180Asset;

	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	TObjectPtr<UAnimSequenceBase> TurnRightLessThan180Asset;

	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	TObjectPtr<UAnimSequenceBase> TurnLeftMoreThan180Asset;

	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	TObjectPtr<UAnimSequenceBase> TurnRightMoreThan180Asset;

	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	FName LegsSlotName;

private:
	bool ShouldRotateInPlace() const;

	bool ShouldDoMovingRotation() const;

	bool IsPlayingTurningMontage() const;

	void SetTargetCharacterRotation();
	
	void UpdateCharacterMovingRotation(float deltaTime);

	void UpdateCharacterRotationThroughCurve(float deltaTime);

	void SetTurnAnimationAsset();

	void SetRotationCurveScaleValue();

	void PlayTurningMontage();

	void StopCurrentlyPlayingTurningMontage();

	bool ShouldDoMontageRotation() const;

	inline void ConvertRotation(float& rotation)
	{
		if (rotation >= 0)
		{
			rotation = -(360 - rotation);
		}
		else
		{
			rotation = 360 + rotation;
		}
	}

	static constexpr float NinetyDegreeRotationCurveAmount = 90;

	static constexpr float OneHundredEightyDegreeRotationCurveAmount = 180;

	static constexpr float TurnAnimationTargetFrameRate = 30;

	float RotationCountdownTimer;

	float CurrCharacterHorizontalAngle;

	float CurrTargetCharacterHorizontalAngle;

	float NextTargetCharacterHorizontalAngle;

	float RotationCurveScaleValue;

	TObjectPtr<UAnimSequenceBase> CurrPlayingTurnSequence;
#pragma endregion

#pragma region Gait
private:
	void OnGaitChangeTriggered(const FInputActionInstance& inputActionInstance);
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

	virtual void OnCameraMovementTriggered(const FInputActionInstance& inputActionInstance);

	float CurrViewVerticalAngle;

	float CurrViewHorizontalAngle;

private:
	void OnCameraMovementStarted(const FInputActionInstance& inputActionInstance);

	void OnCameraMovementComplete(const FInputActionInstance& inputActionInstance);

	void SetTargetCameraRotation(float deltaTime);

	bool HasCameraInput;

	FVector2D CameraInput;	
#pragma endregion

#pragma region Movement and Rotation Info
protected:
	UPROPERTY(EditAnywhere, Category = "Movement")
	FCharacterMovementSettings MovementSettings;

public:
	FCharacterMovementSettings GetMovementSettings() const;

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
