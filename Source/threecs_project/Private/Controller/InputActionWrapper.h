#pragma once

class UInputMappingContext;
class UInputAction;
class UEnhancedInputComponent;

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include <type_traits>
#include "InputActionWrapper.generated.h"

USTRUCT(BlueprintType)
//template<typename ActionStartedDelegate, typename ActionTriggeredDelegate, typename ActionCompletedDelegate>
struct FInputActionWrapper
{
	GENERATED_BODY()
	/*
	static_assert(std::is_base_of<TMulticastDelegate, ActionStartedDelegate>, "ActionStartedDelegate must inherit from TMulticastDelegate");
	static_assert(std::is_base_of<TMulticastDelegate, ActionTriggeredDelegate>, "ActionStartedDelegate must inherit from TMulticastDelegate");
	static_assert(std::is_base_of<TMulticastDelegate, ActionCompletedDelegate>, "ActionStartedDelegate must inherit from TMulticastDelegate");

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> InputAction;

public:
	ActionStartedDelegate ActionStartedEvent;

	ActionTriggeredDelegate ActionTriggeredEvent;

	ActionCompletedDelegate ActionCompletedEvent;

	void BindToInputMap(UEnhancedInputComponent* InputComponent)
	{
		InputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &FInputActionWrapper::OnActionStarted);
		InputComponent->BindAction(InputAction, ETriggerEvent::Triggered, this, &FInputActionWrapper::OnActionTriggered);
		InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &FInputActionWrapper::OnActionCompleted);
	}

private:
	void OnActionStarted(FInputActionInstance& InputAction)
	{
		dynamic_cast<>
	}

	void OnActionTriggered(FInputActionInstance& InputAction);

	void OnActionCompleted(FInputActionInstance& InputAction);
	*/
};