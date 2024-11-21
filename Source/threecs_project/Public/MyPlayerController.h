// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "MyPlayerController.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FVector2Delegate, FVector2D);

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Character Movement")
	class UInputMappingContext* CharacterMovementInputMapping;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Camera")
	class UInputMappingContext* CameraInputMapping;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Character Movement")
	class UInputAction* CharacterMovementInputAction;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Camera")
	class UInputAction* CameraInputAction;

	void CharacterMove(const FInputActionInstance& Instance);

	void CameraMove(const FInputActionInstance& Instance);

public:
	FVector2Delegate OnCharacterMovement;

	FVector2Delegate OnCameraMovement;

	// this is called by any player controller and sets up the default input component
	// that already exists
	virtual void SetupInputComponent() override;
};
