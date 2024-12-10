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
	TObjectPtr<UInputMappingContext> CharacterMovementInputMapping;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Character Movement")
	TObjectPtr<UInputAction> CharacterMovementInputAction;

	void CharacterMovementInputStarted(const FInputActionInstance& Instance);

	void CharacterMovementInputTriggered(const FInputActionInstance& Instance);

	void CharacterMovementInputComplete(const FInputActionInstance& Instance);

public:
	FNoParamsDelegate OnCharacterMovementInputStarted;

	FVector2Delegate OnCharacterMovementInputTriggered;

	FNoParamsDelegate OnCharacterMovementInputComplete;
#pragma endregion

#pragma region Gait
private:
	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Character Movement")
	TObjectPtr<UInputAction> CharacterGaitChangeInputAction;

	void GaitChangeInputStarted(const FInputActionInstance& Instance);

public:
	FNoParamsDelegate OnGaitChangeInputStarted;
#pragma endregion

#pragma region Camera Input
private:
	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Camera")
	TObjectPtr<UInputMappingContext> CameraInputMapping;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Camera")
	TObjectPtr<UInputAction> CameraInputAction;

	void CameraMovementInputStarted(const FInputActionInstance& Instance);

	void CameraMovementInputTriggered(const FInputActionInstance& Instance);

	void CameraMovementInputComplete(const FInputActionInstance& Instance);

public:
	FNoParamsDelegate OnCameraMovementInputStarted;

	FVector2Delegate OnCameraMovementInputTriggered;

	FNoParamsDelegate OnCameraMovementInputComplete;
#pragma endregion
	
public:
	virtual void SetupInputComponent() override;
};
