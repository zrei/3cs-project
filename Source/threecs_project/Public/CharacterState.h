// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterState.generated.h"

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
	IDLE UMETA(DisplayName = "Idle"),
	MOVING UMETA(DisplayName = "Moving")
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
	LEFT UMETA(DisplayName = "Left"),
	RIGHT UMETA(DisplayName = "Right")
};

USTRUCT(BlueprintType)
struct FCharacterState
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	ECharacterMovementState CharacterMovementState;

	UPROPERTY(VisibleAnywhere)
	ERotateDirection RotationDirection;

	UPROPERTY(VisibleAnywhere)
	ECharacterGait CharacterGait;

	FCharacterState() : CharacterMovementState(ECharacterMovementState::IDLE), RotationDirection(ERotateDirection::NONE),
		CharacterGait(ECharacterGait::WALK) {}

	FCharacterState(ECharacterMovementState characterMovementState, ERotateDirection rotateDirection,
		ECharacterGait characterGait) : CharacterMovementState(characterMovementState), RotationDirection(rotateDirection),
		CharacterGait(characterGait) {}
};

USTRUCT(BlueprintType)
struct FCharacterMovementSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	float FastRotationSpeed;

	UPROPERTY(VisibleAnywhere)
	float SlowRotationSpeed;

	UPROPERTY(VisibleAnywhere)
	float LargeRotationThreshold;

	FCharacterMovementSettings() : FastRotationSpeed(0), SlowRotationSpeed(0),
		LargeRotationThreshold(0) {
	}

	FCharacterMovementSettings(float fastRotationSpeed, float slowRotationSpeed, float largeRotationThreshold)
		: FastRotationSpeed(fastRotationSpeed), SlowRotationSpeed(slowRotationSpeed), LargeRotationThreshold(LargeRotationThreshold) {
	}
};
