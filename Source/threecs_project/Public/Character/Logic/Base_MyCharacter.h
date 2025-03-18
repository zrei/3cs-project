// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UCameraComponent;
class USceneComponent;
struct FInputActionInstance;
class UCharacterTurnAnimationSettings;
class UCharacterLocomotionSettings;
class UCharacterCameraSettings;
class AMyPlayerController;

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Character/Logic/CharacterState.h"
#include "UI/ControlScheme.h"
#include "Base_MyCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FControlSchemeDelegate, EControlScheme);

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

#pragma region Locomotion
	void SubscribeToLocomotionInputs(AMyPlayerController* const playerController);

	void UnsubscribeToLocomotionInputs(AMyPlayerController* const playerController);
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
#pragma endregion

#pragma region Rotation
protected:
	UPROPERTY(EditAnywhere, Category = "RotationAnimation")
	TObjectPtr<UCharacterTurnAnimationSettings> NormalTurnAnimationSettings;

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

	static constexpr float NinetyDegreeRotationCurveAmount = 90;

	static constexpr float OneHundredEightyDegreeRotationCurveAmount = 180;

	static constexpr float TurnAnimationTargetFrameRate = 30;

	static constexpr float MovingTurnStartTime = 0.33;

	float RotationCountdownTimer;

	float RotationCurveScaleValue;

	TObjectPtr<UAnimSequenceBase> CurrPlayingTurnSequence;

public:
	/// <summary>
	/// Returns the shortest angular distance from the currRotation to targetRotation
	/// </summary>
	/// <param name="currRotation">Should be between -180 and 180</param>
	/// <param name="targetRotation">Should be between -180 and 180</param>
	/// <returns></returns>
	static inline float CalculateShortestRotationDiff(float currRotationYaw, float targetRotationYaw)
	{
		// convert to 0-360 for easier comparison
		float convertedCurrYaw = currRotationYaw + 180;
		float convertedTargetYaw = targetRotationYaw + 180;
		float diff = convertedTargetYaw - convertedCurrYaw;

		if (diff > 180)
		{
			return -(360 - diff);
		}
		else if (diff < -180)
		{
			return 360 + diff;
		}
		else
		{
			return diff;
		}
	}
#pragma endregion

#pragma region Gait
private:
	void OnGaitChangeTriggered(const FInputActionInstance& inputActionInstance);
#pragma endregion

#pragma region Camera
protected:
	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<UCharacterCameraSettings> CharacterCameraSettings;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<USceneComponent> CameraParent;

	virtual void RotateCamera() PURE_VIRTUAL(ABase_MyCharacter::RotateCamera)

	virtual void OnCameraMovementTriggered(const FInputActionInstance& inputActionInstance);

public:
	const FCameraSettings& GetCameraSettings() const;

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
	TObjectPtr<UCharacterLocomotionSettings> CharacterLocomotionSettings;

public:
	const FCharacterMovementSettings& GetMovementSettings() const;

	const FCharacterState& GetCurrentState() const;

protected:
	FCharacterState CurrCharacterState;
#pragma endregion

#pragma region Skeleton and Animation
private:
	TObjectPtr<UAnimInstance> MainAnimInstance;
#pragma endregion

#pragma region Rope swing
public:
	/// <summary>
	/// Returns if the character can enter swing state
	/// </summary>
	/// <returns></returns>
	bool EnterSwingState();

	bool ExitSwingState();

	void UpdateHandPositions(FVector left, FVector right);
#pragma endregion

public:
	FControlSchemeDelegate ControlSchemeChangedEvent;
};
