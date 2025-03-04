// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UVerticalBox;
class ARope;

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interactables/RopeState.h"
#include "ControlsDisplayWidget.generated.h"

enum class EControlScheme : uint8
{
	NORMAL,
	ROPE_SWING,
	ROPE_SHIMMY
};

/**
 * 
 */
UCLASS()
class THREECS_PROJECT_API UControlsDisplayWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> NormalControlsPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> RopeSwingControlsPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> RopeShimmyControlsPanel;

	virtual void NativeConstruct() override;

	virtual void NativeOnInitialized() override;

	virtual void NativeDestruct() override;

private:
	void OnRopeAttach(ARope* const);

	void OnRopeDetach();

	void OnRopeStateToggle(ERopeInputState);

	void ToggleVisibility(EControlScheme);
};
