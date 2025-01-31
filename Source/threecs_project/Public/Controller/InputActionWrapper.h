#pragma once

//class UInputAction;
//class UEnhancedInputComponent;

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "InputActionWrapper.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FInputActionDelegate, const FInputActionInstance&);

UENUM(BlueprintType)
enum class FInputType : uint8
{
	CAMERA_MOVEMENT UMETA(DisplayName = "Camera - Movement"),
	LOCOMOTION_JUMPING UMETA(DisplayName = "Locomotion - Jumping"),
	LOCOMOTION_MOVEMENT UMETA(DisplayName = "Locomotion - Movement"),
	LOCOMOTION_GAIT UMETA(DisplayName = "Locomotion - Gait")
};

USTRUCT(BlueprintType)
struct FInputActionWrapper
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditAnywhere)
	FInputType InputType;

public:
	FInputActionWrapper() : HasBeenBinded(false) {}

	FInputType GetInputType() const
	{
		return InputType;
	}

public:
	FInputActionDelegate ActionStartedEvent;

	FInputActionDelegate ActionTriggeredEvent;

	FInputActionDelegate ActionCompletedEvent;

	FInputActionDelegate ActionOngoingEvent;

	FInputActionDelegate ActionCancelledEvent;

private:
	uint32 ActionStartedBindingHandle;

	uint32 ActionTriggeredBindingHandle;

	uint32 ActionCompletedBindingHandle;

	uint32 ActionOngoingBindingHandle;

	uint32 ActionCancelledBindingHandle;

	bool HasBeenBinded;

public:
	void BindToInput(UEnhancedInputComponent* InputComponent)
	{
		if (HasBeenBinded)
			return;
		ActionStartedBindingHandle = InputComponent->BindActionInstanceLambda(InputAction, ETriggerEvent::Started, [this](const FInputActionInstance& inputAction) {this->ActionStartedEvent.Broadcast(inputAction);}).GetHandle();
		ActionCompletedBindingHandle = InputComponent->BindActionInstanceLambda(InputAction, ETriggerEvent::Completed, [this](const FInputActionInstance& inputAction) {this->ActionCompletedEvent.Broadcast(inputAction);}).GetHandle();
		ActionTriggeredBindingHandle = InputComponent->BindActionInstanceLambda(InputAction, ETriggerEvent::Triggered, [this](const FInputActionInstance& inputAction) {this->ActionTriggeredEvent.Broadcast(inputAction);}).GetHandle();
		ActionCancelledBindingHandle = InputComponent->BindActionInstanceLambda(InputAction, ETriggerEvent::Canceled, [this](const FInputActionInstance& inputAction) {this->ActionCancelledEvent.Broadcast(inputAction);}).GetHandle();
		ActionOngoingBindingHandle = InputComponent->BindActionInstanceLambda(InputAction, ETriggerEvent::Ongoing, [this](const FInputActionInstance& inputAction) {this->ActionOngoingEvent.Broadcast(inputAction);}).GetHandle();
		HasBeenBinded = true;
	}

	void UnbindToInput(UEnhancedInputComponent* InputComponent)
	{
		if (!HasBeenBinded)
			return;
		InputComponent->RemoveBindingByHandle(ActionStartedBindingHandle);
		InputComponent->RemoveBindingByHandle(ActionTriggeredBindingHandle);
		InputComponent->RemoveBindingByHandle(ActionCompletedBindingHandle);
		InputComponent->RemoveBindingByHandle(ActionOngoingBindingHandle);
		InputComponent->RemoveBindingByHandle(ActionCancelledBindingHandle);
		HasBeenBinded = false;
	}
};