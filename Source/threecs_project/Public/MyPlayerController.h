// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UInputMappingContext;
class UInputAction;

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "MyPlayerController.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FVector2Delegate, FVector2D);
DECLARE_MULTICAST_DELEGATE_OneParam(FBoolDelegate, bool);
DECLARE_MULTICAST_DELEGATE(FNoParamsDelegate);

/**
 * Handles inputs and broadcasts them
 */
UCLASS()
class THREECS_PROJECT_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

#pragma region Character Movement
private:
	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Character Movement")
	UInputMappingContext* CharacterMovementInputMapping;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Character Movement")
	UInputAction* CharacterMovementInputAction;

	void CharacterMovementStarted(const FInputActionInstance& Instance);

	void CharacterMove(const FInputActionInstance& Instance);

	void CharacterMovementComplete(const FInputActionInstance& Instance);

public:
	FNoParamsDelegate OnCharacterMovementStarted;

	FVector2Delegate OnCharacterMovement;

	FNoParamsDelegate OnCharacterMovementComplete;
#pragma endregion

#pragma region Gait
private:
	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Character Movement")
	UInputAction* CharacterGaitChangeInputAction;

	void GaitChange(const FInputActionInstance& Instance);

public:
	FBoolDelegate OnGaitChange;
#pragma endregion

#pragma region Camera Input
private:
	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Camera")
	UInputMappingContext* CameraInputMapping;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Camera")
	UInputAction* CameraInputAction;

	void CameraMovementStarted(const FInputActionInstance& Instance);

	void CameraMove(const FInputActionInstance& Instance);

	void CameraMovementComplete(const FInputActionInstance& Instance);

public:
	FNoParamsDelegate OnCameraMovementStarted;

	FVector2Delegate OnCameraMovement;

	FNoParamsDelegate OnCameraMovementComplete;
#pragma endregion
	
public:
	virtual void SetupInputComponent() override;
};
