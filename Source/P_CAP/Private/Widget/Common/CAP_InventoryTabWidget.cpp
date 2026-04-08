// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_InventoryTabWidget.h"

#include "Widget/Item/CAP_ItemEquipPanelWidget.h"

void UCAP_InventoryTabWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCAP_InventoryTabWidget::RefreshInventoryTab(ACAP_PlayerCharacter* PlayerCharacter)
{
	if (ItemEquipPanel)
	{
		ItemEquipPanel->RefreshPanel(PlayerCharacter);
	}
}
