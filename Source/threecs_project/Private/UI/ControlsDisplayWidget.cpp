// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ControlsDisplayWidget.h"
#include "Components/VerticalBox.h"
#include "Interactables/Rope.h"

void UControlsDisplayWidget::OnRopeAttach(ARope* const)
{
	ToggleVisibility(EControlScheme::ROPE_SWING);
}

void UControlsDisplayWidget::OnRopeDetach()
{
	ToggleVisibility(EControlScheme::NORMAL);
}

void UControlsDisplayWidget::OnRopeStateToggle(ERopeInputState ropeInputState)
{
	ToggleVisibility(ropeInputState == ERopeInputState::SHIMMY ? EControlScheme::ROPE_SHIMMY : EControlScheme::ROPE_SWING);
}

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
	ARope::RopeAttachEvent.AddUObject(this, &UControlsDisplayWidget::OnRopeAttach);
	ARope::RopeDetachEvent.AddUObject(this, &UControlsDisplayWidget::OnRopeDetach);
	ARope::RopeStateToggleEvent.AddUObject(this, &UControlsDisplayWidget::OnRopeStateToggle);
}

void UControlsDisplayWidget::NativeDestruct()
{
	ARope::RopeAttachEvent.RemoveAll(this);
	ARope::RopeDetachEvent.RemoveAll(this);
	ARope::RopeStateToggleEvent.RemoveAll(this);
}