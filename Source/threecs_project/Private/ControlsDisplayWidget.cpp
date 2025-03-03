// Fill out your copyright notice in the Description page of Project Settings.


#include "ControlsDisplayWidget.h"
#include "Components/VerticalBox.h"
#include "Rope.h"

void UControlsDisplayWidget::OnRopeAttach(ARope* const)
{
	ToggleVisibility(EControlScheme::ROPE_SWING);
}

void UControlsDisplayWidget::OnRopeDetach()
{
	ToggleVisibility(EControlScheme::NORMAL);
}

void UControlsDisplayWidget::ToggleVisibility(EControlScheme controlScheme)
{
	NormalControlsPanel->SetVisibility(controlScheme == EControlScheme::NORMAL ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	SwingControlsPanel->SetVisibility(controlScheme == EControlScheme::ROPE_SWING ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UControlsDisplayWidget::NativeConstruct()
{
	ToggleVisibility(EControlScheme::NORMAL);
}

void UControlsDisplayWidget::NativeOnInitialized()
{
	ARope::RopeAttachEvent.AddUObject(this, &UControlsDisplayWidget::OnRopeAttach);
	ARope::RopeDetachEvent.AddUObject(this, &UControlsDisplayWidget::OnRopeDetach);
}

void UControlsDisplayWidget::NativeDestruct()
{
	ARope::RopeAttachEvent.RemoveAll(this);
	ARope::RopeDetachEvent.RemoveAll(this);
}