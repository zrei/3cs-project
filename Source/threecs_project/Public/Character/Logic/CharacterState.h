// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/RopeState.h"
#include "CharacterState.generated.h"

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
	IDLE UMETA(DisplayName = "Idle"),
	MOVING UMETA(DisplayName = "Moving"),
	JUMPING UMETA(DisplayName = "Jumping"),
	SWINGING UMETA(DisplayName = "Swinging"),
	EXIT_SWINGING UMETA(DisplayName = "ExitSwinging")
};

UENUM(BlueprintType)
enum class ECharacterGait : uint8
{
	WALK UMETA(DisplayName = "Walk"),
	RUN  UMETA(DisplayName = "Run")
};

UENUM(BlueprintType)
enum class ERotateDirection : uint8
{
	NONE UMETA(DisplayName = "None"),
	RIGHT UMETA(DisplayName = "Right"),
	LEFT UMETA(DisplayName = "Left")
};

USTRUCT(BlueprintType)
struct FCharacterState
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D MovementInput;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECharacterMovementState CharacterMovementState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ERotateDirection CurrRotationDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ERotateDirection NextRotationDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECharacterGait CharacterGait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TargetCharacterRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float NextTargetCharacterRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrCharacterRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrLookYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrLookPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrCharacterSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TargetCharacterSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LeftHandPosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector RightHandPosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool EnableHandIK;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ERopeInputState RopeInputState;

	FCharacterState() : MovementInput(0, 0), CharacterMovementState(ECharacterMovementState::IDLE), CurrRotationDirection(ERotateDirection::NONE), NextRotationDirection(ERotateDirection::NONE),
		CharacterGait(ECharacterGait::WALK), TargetCharacterRotation(0), NextTargetCharacterRotation(0), CurrCharacterRotation(0), CurrLookYaw(0), CurrLookPitch(0),
		CurrCharacterSpeed(0), TargetCharacterSpeed(0), LeftHandPosition(), RightHandPosition(), EnableHandIK(false), RopeInputState(ERopeInputState::SWING) { }

	FCharacterState(FVector2D movementInput, ECharacterMovementState characterMovementState, ERotateDirection currRotateDirection, ERotateDirection nextRotateDirection,
		ECharacterGait characterGait, float targetCharacterRotation, float nextTargetCharacterRotation, float currCharacterRotation, float currLookYaw, float currLookPitch,
		float currCharacterSpeed, float targetCharacterSpeed, FVector leftHandPosition, FVector rightHandPosition, bool enableHandIK, ERopeInputState ropeInputState) : MovementInput(movementInput), CharacterMovementState(characterMovementState), CurrRotationDirection(currRotateDirection),
		NextRotationDirection(nextRotateDirection), CharacterGait(characterGait), TargetCharacterRotation(targetCharacterRotation), NextTargetCharacterRotation(nextTargetCharacterRotation),
		CurrCharacterRotation(currCharacterRotation), CurrLookYaw(currLookYaw), CurrLookPitch(currLookPitch), CurrCharacterSpeed(currCharacterSpeed),
		TargetCharacterSpeed(targetCharacterSpeed), LeftHandPosition(leftHandPosition), RightHandPosition(rightHandPosition), EnableHandIK(enableHandIK), RopeInputState(ropeInputState) {}
};

USTRUCT(BlueprintType)
struct FCharacterMovementSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CharacterWalkMovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CharacterRunMovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CharacterAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CharacterDecceleration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CharacterStationaryRotationalSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CharacterMovingRotationalSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RotationAngleThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RotationLookTimeThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MovingRotationSpeedPlayRateScale = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinimumTurnMontageSpeed = 1;

	FCharacterMovementSettings() : CharacterWalkMovementSpeed(0), CharacterRunMovementSpeed(0), CharacterAcceleration(0), CharacterDecceleration(0),
		CharacterStationaryRotationalSpeed(0), CharacterMovingRotationalSpeed(0), RotationAngleThreshold(0), RotationLookTimeThreshold(0),
		MovingRotationSpeedPlayRateScale(0), MinimumTurnMontageSpeed(0) {}

	FCharacterMovementSettings(float characterWalkMovementSpeed, float characterRunMovementSpeed, float characterAcceleration, float characterDecceleration,
		float characterStationaryRotationalSpeed, float characterMovingRotationalSpeed, float rotationAngleThreshold, float rotationLookTimeThreshold,
		float movingRotationSpeedPlayRateScale, float minimumTurnMontageSpeed) : CharacterWalkMovementSpeed(characterWalkMovementSpeed), CharacterRunMovementSpeed(characterRunMovementSpeed),
		CharacterAcceleration(characterAcceleration), CharacterDecceleration(characterDecceleration), CharacterStationaryRotationalSpeed(characterStationaryRotationalSpeed),
		CharacterMovingRotationalSpeed(characterMovingRotationalSpeed), RotationAngleThreshold(rotationAngleThreshold), RotationLookTimeThreshold(rotationLookTimeThreshold),
		MovingRotationSpeedPlayRateScale(movingRotationSpeedPlayRateScale), MinimumTurnMontageSpeed(minimumTurnMontageSpeed) {}
};

USTRUCT(BlueprintType)
struct FCameraSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float MinViewVerticalAngle;

	UPROPERTY(EditAnywhere)
	float MaxViewVerticalAngle;

	UPROPERTY(EditAnywhere)
	float CameraRotationalSpeed;

	FCameraSettings() : MinViewVerticalAngle(0), MaxViewVerticalAngle(0), CameraRotationalSpeed(0) {}

	FCameraSettings(float minViewVerticalAngle, float maxViewVerticalAngle, float cameraRotationalSpeed) :
		MinViewVerticalAngle(minViewVerticalAngle), MaxViewVerticalAngle(maxViewVerticalAngle), CameraRotationalSpeed(cameraRotationalSpeed) {}
};