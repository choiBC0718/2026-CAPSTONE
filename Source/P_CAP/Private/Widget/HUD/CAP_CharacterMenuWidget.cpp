// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_CharacterMenuWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/WidgetSwitcher.h"
#include "Widget/Common/CAP_InventoryTabWidget.h"

void UCAP_CharacterMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCAP_CharacterMenuWidget::RefreshMenu()
{
	if (InventoryTabWidget)
	{
		if (ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn()))
		{
			InventoryTabWidget->RefreshInventoryTab(PlayerCharacter);
		}
	}
}

void UCAP_CharacterMenuWidget::SwitchNextTab()
{
	if (WidgetSwitcher)
	{
		int32 NextIndex = (WidgetSwitcher->GetActiveWidgetIndex() +1 ) % WidgetSwitcher->GetNumWidgets();
		WidgetSwitcher->SetActiveWidgetIndex(NextIndex);
	}
}
