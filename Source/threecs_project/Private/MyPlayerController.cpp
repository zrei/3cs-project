// Fill out your copyright notice in the Description page of Project Settings.


#include "threecs_project/Public/MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// make a note about input components and setup input component
void AMyPlayerController::SetupInputComponent()
{
	// parent
	Super::SetupInputComponent();

	// subscribe to input mappings
	ULocalPlayer* localPlayer = GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(localPlayer);
	if (Subsystem)
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(CharacterMovementInputMapping, 0);
		Subsystem->AddMappingContext(CameraInputMapping, 1);
	}

	// subscribe input actions to the pre-existing input component
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	Input->BindAction(CharacterMovementInputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::CharacterMove);
	Input->BindAction(CameraInputAction, ETriggerEvent::Triggered, this, &AMyPlayerController::CameraMove);
}

void AMyPlayerController::CharacterMove(const FInputActionInstance& Instance)
{
	// execute if bound??? uh
	// unwrap wrapping and get the vector 2d value, and fire the delegate
	UE_LOG(LogTemp, Warning, TEXT("Character movement from controller"));
	OnCharacterMovement.Broadcast(Instance.GetValue().Get<FVector2D>());
}

void AMyPlayerController::CameraMove(const FInputActionInstance& Instance)
{
	UE_LOG(LogTemp, Warning, TEXT("Camera movement from controller"));
	OnCameraMovement.Broadcast(Instance.GetValue().Get<FVector2D>());
}