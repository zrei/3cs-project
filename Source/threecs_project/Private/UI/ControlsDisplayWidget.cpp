// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ControlsDisplayWidget.h"
#include "Components/VerticalBox.h"
#include "Character/Logic/Base_MyCharacter.h"
#include "Kismet/GameplayStatics.h"

void UControlsDisplayWidget::ToggleVisibility(EControlScheme controlScheme)
{
	NormalControlsPanel->SetVisibility(controlScheme == EControlScheme::NORMAL ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	RopeSwingControlsPanel->SetVisibility(controlScheme == EControlScheme::ROPE_SWING ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	RopeShimmyControlsPanel->SetVisibility(controlScheme == EControlScheme::ROPE_SHIMMY ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UControlsDisplayWidget::NativeConstruct()
{
	ToggleVisibility(EControlScheme::NORMAL);
}

void UControlsDisplayWidget::NativeOnInitialized()
{
	Character = Cast<ABase_MyCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (Character)
		Character->ControlSchemeChangedEvent.AddUObject(this, &UControlsDisplayWidget::ToggleVisibility);
}

void UControlsDisplayWidget::NativeDestruct()
{
	if (Character)
		Character->ControlSchemeChangedEvent.RemoveAll(this);
}
