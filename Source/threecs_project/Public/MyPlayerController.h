// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UInputMappingContext;
class UInputAction;

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
	UInputMappingContext* CharacterMovementInputMapping;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Camera")
	UInputMappingContext* CameraInputMapping;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Character Movement")
	UInputAction* CharacterMovementInputAction;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input - Camera")
	UInputAction* CameraInputAction;

	void CharacterMove(const FInputActionInstance& Instance);

	void CameraMove(const FInputActionInstance& Instance);

public:
	FVector2Delegate OnCharacterMovement;

	FVector2Delegate OnCameraMovement;

	virtual void SetupInputComponent() override;
};
