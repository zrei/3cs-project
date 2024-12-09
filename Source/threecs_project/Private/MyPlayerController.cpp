// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/MyPlayerController.h"
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

	Input->BindAction(CharacterMovementInputAction, ETriggerEvent::Started, this, &AMyPlayerController::CharacterMovementStarted);
	Input->BindAction(CharacterMovementInputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::CharacterMove);
	Input->BindAction(CharacterMovementInputAction, ETriggerEvent::Completed, this, &AMyPlayerController::CharacterMovementComplete);
	
	Input->BindAction(CameraInputAction, ETriggerEvent::Started, this, &AMyPlayerController::CameraMovementStarted);
	Input->BindAction(CameraInputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::CameraMove);
	Input->BindAction(CameraInputAction, ETriggerEvent::Completed, this, &AMyPlayerController::CameraMovementComplete);
	
	Input->BindAction(CharacterGaitChangeInputAction, ETriggerEvent::Started, this, &AMyPlayerController::GaitChange);
}

#pragma region Character Movement
void AMyPlayerController::CharacterMovementStarted(const FInputActionInstance& Instance)
{
	OnCharacterMovementStarted.Broadcast();
}

void AMyPlayerController::CharacterMove(const FInputActionInstance& Instance)
{
	OnCharacterMovement.Broadcast(Instance.GetValue().Get<FVector2D>());
}

void AMyPlayerController::CharacterMovementComplete(const FInputActionInstance& Instance)
{
	OnCharacterMovementComplete.Broadcast();
}
#pragma endregion

#pragma region Camera Input
void AMyPlayerController::CameraMovementStarted(const FInputActionInstance& Instance)
{
	OnCameraMovementStarted.Broadcast();
}

void AMyPlayerController::CameraMove(const FInputActionInstance& Instance)
{
	OnCameraMovement.Broadcast(Instance.GetValue().Get<FVector2D>());
}

void AMyPlayerController::CameraMovementComplete(const FInputActionInstance& Instance)
{
	OnCameraMovementComplete.Broadcast();
}
#pragma endregion

#pragma region Gait
void AMyPlayerController::GaitChange(const FInputActionInstance& Instance)
{
	OnGaitChange.Broadcast(Instance.GetValue().Get<bool>());
}
#pragma endregion
