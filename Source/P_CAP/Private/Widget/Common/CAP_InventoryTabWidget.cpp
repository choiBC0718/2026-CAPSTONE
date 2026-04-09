// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_InventoryTabWidget.h"

#include "Widget/Item/CAP_ItemDetailPanelWidget.h"
#include "Widget/Item/CAP_ItemEquipPanelWidget.h"

void UCAP_InventoryTabWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ItemEquipPanel)
	{
		// 슬롯 클릭 됨
		ItemEquipPanel->OnPanelSlotClicked.AddUObject(this, &UCAP_InventoryTabWidget::OnItemSlotClicked);
	}
}

void UCAP_InventoryTabWidget::RefreshInventoryTab(ACAP_PlayerCharacter* PlayerCharacter)
{
	if (ItemEquipPanel)
	{
		ItemEquipPanel->RefreshPanel(PlayerCharacter);
	}
}

void UCAP_InventoryTabWidget::NavigationInput(FVector2D InputVal)
{
	if (ItemEquipPanel)
	{
		ItemEquipPanel->MoveSelection(InputVal);
	}
}


void UCAP_InventoryTabWidget::OnItemSlotClicked(class UCAP_ItemSlotWidget* ClickedSlot)
{
	if (ItemDetailPanel && ClickedSlot)
	{
		ItemDetailPanel->UpdateDetailInfo(ClickedSlot->SlotItemData, ClickedSlot->SlotType);
	}
}
