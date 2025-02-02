// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UInputMappingContext;
class UInputAction;

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputActionWrapper.h"
#include "MyPlayerController.generated.h"

/**
 * Handles inputs and broadcasts them
 */
UCLASS()
class THREECS_PROJECT_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Enhanced Input")
	TArray<TObjectPtr<UInputMappingContext>> MappingContexts;

	UPROPERTY(EditAnywhere, Category = "Enhanced Input")
	TArray<FInputActionWrapper> ActionInputWrappers;

public:
	FInputActionWrapper& GetActionInputWrapper(FInputType inputType);
	
public:
	virtual void SetupInputComponent() override;
};
