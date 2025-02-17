// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterState.generated.h"

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
	IDLE UMETA(DisplayName = "Idle"),
	MOVING UMETA(DisplayName = "Moving"),
	JUMPING UMETA(DisplayName = "Jumping")
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
	ECharacterMovementState CharacterMovementState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ERotateDirection RotationDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECharacterGait CharacterGait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TargetCharacterRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrCharacterRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrLookPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrCharacterSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TargetCharacterSpeed;

	FCharacterState() : CharacterMovementState(ECharacterMovementState::IDLE), RotationDirection(ERotateDirection::NONE),
		CharacterGait(ECharacterGait::WALK), TargetCharacterRotation(0), CurrCharacterRotation(0), CurrLookPitch(0), CurrCharacterSpeed(0),
		TargetCharacterSpeed(0) {}

	FCharacterState(ECharacterMovementState characterMovementState, ERotateDirection rotateDirection,
		ECharacterGait characterGait, float targetCharacterRotation, float currCharacterRotation, float currLookPitch, float currCharacterSpeed,
		float targetCharacterSpeed) : CharacterMovementState(characterMovementState), RotationDirection(rotateDirection),
		CharacterGait(characterGait), TargetCharacterRotation(targetCharacterRotation), CurrCharacterRotation(currCharacterRotation), CurrLookPitch(currLookPitch),
		CurrCharacterSpeed(currCharacterSpeed), TargetCharacterSpeed(targetCharacterSpeed) {}
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
	float MovingRotationTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RotationAngleThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RotationLookTimeThreshold;

	FCharacterMovementSettings() : CharacterWalkMovementSpeed(0), CharacterRunMovementSpeed(0), CharacterAcceleration(0), CharacterDecceleration(0),
		CharacterStationaryRotationalSpeed(0), CharacterMovingRotationalSpeed(0), MovingRotationTime(0), RotationAngleThreshold(0), RotationLookTimeThreshold(0) {}

	FCharacterMovementSettings(float characterWalkMovementSpeed, float characterRunMovementSpeed, float characterAcceleration, float characterDecceleration,
		float characterStationaryRotationalSpeed, float characterMovingRotationalSpeed, float movingRotationTime, float rotationAngleThreshold, float rotationLookTimeThreshold)
		: CharacterWalkMovementSpeed(characterWalkMovementSpeed), CharacterRunMovementSpeed(characterRunMovementSpeed),
		CharacterAcceleration(characterAcceleration), CharacterDecceleration(characterDecceleration), CharacterStationaryRotationalSpeed(characterStationaryRotationalSpeed),
		CharacterMovingRotationalSpeed(characterMovingRotationalSpeed), MovingRotationTime(movingRotationTime), RotationAngleThreshold(rotationAngleThreshold), RotationLookTimeThreshold(rotationLookTimeThreshold) {}
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