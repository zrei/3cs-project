// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UVerticalBox;
class ABase_MyCharacter;

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interactables/RopeState.h"
#include "UI/ControlScheme.h"
#include "ControlsDisplayWidget.generated.h"

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
	void ToggleVisibility(EControlScheme controlScheme);

	TObjectPtr<ABase_MyCharacter> Character;
};
