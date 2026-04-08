// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_ItemSlotWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"

void UCAP_ItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotButton)
	{
		SlotButton->OnClicked.AddDynamic(this, &UCAP_ItemSlotWidget::HandleSlotClicked);
	}
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


void UCAP_ItemSlotWidget::HandleSlotClicked()
{
	if (SlotButton)
		SlotButton->SetKeyboardFocus();
}

void UCAP_ItemSlotWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	OnSlotFocused.Broadcast(this);
}
