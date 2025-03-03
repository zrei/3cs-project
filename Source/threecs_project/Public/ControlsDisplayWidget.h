// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UVerticalBox;
class ARope;

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ControlsDisplayWidget.generated.h"

enum class EControlScheme : uint8
{
	NORMAL,
	ROPE_SWING
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
	TObjectPtr<UVerticalBox> SwingControlsPanel;

	virtual void NativeConstruct() override;

	virtual void NativeOnInitialized() override;

	virtual void NativeDestruct() override;

private:
	void OnRopeAttach(ARope* const);

	void OnRopeDetach();

	void ToggleVisibility(EControlScheme);
};
