// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	ULocalPlayer* localPlayer = GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(localPlayer);
	if (Subsystem)
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(CharacterMovementInputMapping, 0);
		Subsystem->AddMappingContext(CameraInputMapping, 1);
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);

	Input->BindAction(CharacterMovementInputAction, ETriggerEvent::Started, this, &AMyPlayerController::CharacterMovementInputStarted);
	Input->BindAction(CharacterMovementInputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::CharacterMovementInputTriggered);
	Input->BindAction(CharacterMovementInputAction, ETriggerEvent::Completed, this, &AMyPlayerController::CharacterMovementInputComplete);
	
	Input->BindAction(CameraInputAction, ETriggerEvent::Started, this, &AMyPlayerController::CameraMovementInputStarted);
	Input->BindAction(CameraInputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::CameraMovementInputTriggered);
	Input->BindAction(CameraInputAction, ETriggerEvent::Completed, this, &AMyPlayerController::CameraMovementInputComplete);

	Input->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::CharacterJumpInputTriggered);
	
	Input->BindAction(CharacterGaitChangeInputAction, ETriggerEvent::Started, this, &AMyPlayerController::GaitChangeInputStarted);
}

#pragma region Character Movement
void AMyPlayerController::CharacterMovementInputStarted(const FInputActionInstance& Instance)
{
	OnCharacterMovementInputStarted.Broadcast();
}

void AMyPlayerController::CharacterMovementInputTriggered(const FInputActionInstance& Instance)
{
	const FVector2D& movementInput = Instance.GetValue().Get<FVector2D>();
	const FVector2D normalisedMovementInput = movementInput.GetSafeNormal();
	OnCharacterMovementInputTriggered.Broadcast(normalisedMovementInput);
}

void AMyPlayerController::CharacterMovementInputComplete(const FInputActionInstance& Instance)
{
	OnCharacterMovementInputComplete.Broadcast();
}

void AMyPlayerController::CharacterJumpInputTriggered(const FInputActionInstance& Instance)
{
	OnJumpInputTriggered.Broadcast();
}
#pragma endregion

#pragma region Camera Input
void AMyPlayerController::CameraMovementInputStarted(const FInputActionInstance& Instance)
{
	OnCameraMovementInputStarted.Broadcast();
}

void AMyPlayerController::CameraMovementInputTriggered(const FInputActionInstance& Instance)
{
	const FVector2D& cameraInput = Instance.GetValue().Get<FVector2D>();
	OnCameraMovementInputTriggered.Broadcast(cameraInput);
}

void AMyPlayerController::CameraMovementInputComplete(const FInputActionInstance& Instance)
{
	OnCameraMovementInputComplete.Broadcast();
}
#pragma endregion

#pragma region Gait
void AMyPlayerController::GaitChangeInputStarted(const FInputActionInstance& Instance)
{
	OnGaitChangeInputStarted.Broadcast();
}
#pragma endregion
