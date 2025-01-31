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
		for (size_t i = 0; i < MappingContexts.Num(); ++i)
		{
			Subsystem->AddMappingContext(MappingContexts[i], i);
		}
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);

	for (FInputActionWrapper& inputActionWrapper : ActionInputWrappers)
	{
		inputActionWrapper.BindToInput(Input);
	}
}

FInputActionWrapper& AMyPlayerController::GetActionInputWrapper(FInputType inputType)
{
	for (FInputActionWrapper& actionInputWrapper : ActionInputWrappers)
	{
		if (actionInputWrapper.GetInputType() == inputType)
		{
			return actionInputWrapper;
		}
			
	}
	return ActionInputWrappers[0];
}
