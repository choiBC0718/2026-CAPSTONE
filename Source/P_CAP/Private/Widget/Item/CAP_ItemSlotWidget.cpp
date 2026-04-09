// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_ItemSlotWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"

void UCAP_ItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	
}

void UCAP_ItemSlotWidget::InitSlot(ESlotItemType InSlotType, UTexture2D* InIcon, UObject* InItemData)
{
	SlotType = InSlotType;
	SlotItemData = InItemData;
	if (ItemIcon && InIcon)
	{
		ItemIcon->SetBrushFromTexture(InIcon);
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCAP_ItemSlotWidget::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber = NewSlotNumber;
}



void UCAP_ItemSlotWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
}

FReply UCAP_ItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	FReply SuperReply = Super::NativeOnMouseButtonDown(Geometry, MouseEvent);
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		return FReply::Handled().SetUserFocus(TakeWidget());
	}
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().SetUserFocus(TakeWidget()).DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return SuperReply;
}

FReply UCAP_ItemSlotWidget::NativeOnMouseButtonUp(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	FReply SuperReply = Super::NativeOnMouseButtonUp(Geometry, MouseEvent);
	if (HasAnyUserFocus())
	{
		if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			RightButtonClicked();
			return FReply::Handled();
		}
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			LeftButtonClicked();
			return FReply::Handled();
		}
	}
	return SuperReply;
}

void UCAP_ItemSlotWidget::RightButtonClicked()
{
	OnRightMouseClick.Broadcast(this);
}

void UCAP_ItemSlotWidget::LeftButtonClicked()
{
	OnLeftMouseClick.Broadcast(this);
}
