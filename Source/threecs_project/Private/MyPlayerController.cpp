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
	Input->BindAction(CharacterMovementInputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::CharacterMove);
	Input->BindAction(CameraInputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::CameraMove);
}

void AMyPlayerController::CharacterMove(const FInputActionInstance& Instance)
{
	OnCharacterMovement.Broadcast(Instance.GetValue().Get<FVector2D>());
}

void AMyPlayerController::CameraMove(const FInputActionInstance& Instance)
{
	OnCameraMovement.Broadcast(Instance.GetValue().Get<FVector2D>());
}