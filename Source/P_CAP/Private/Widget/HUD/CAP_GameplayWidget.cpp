// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_GameplayWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAP_CharacterMenuWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/WidgetSwitcher.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Items/Weapon/CAP_WeaponComponent.h"
#include "Widget/Common/CAP_ValueGauge.h"
#include "Widget/Common/CAP_AbilityListView.h"
#include "Widget/PanelWidgets/CAP_ItemSwapWidget.h"

void UCAP_GameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player)
		return;
	
	if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
	{
		// 무기 변경 델리게이트 연결
		WeaponComp->OnWeaponChanged.AddDynamic(this, &UCAP_GameplayWidget::HandleWeaponChanged);
	}
	
	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);
		
	if (OwnerASC && HealthBar)
	{
		HealthBar->SetAndBoundToGameplayAttribute(OwnerASC, UCAP_AttributeSet::GetHealthAttribute(), UCAP_AttributeSet::GetMaxHealthAttribute());
	}
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MenuSwitcher)
	{
		MenuSwitcher->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool UCAP_GameplayWidget::IsCharacterMenuOpen()
{
	return MenuSwitcher && MenuSwitcher->GetVisibility() == ESlateVisibility::Visible && MenuSwitcher->GetActiveWidget() == CharacterMenuWidget;
}

bool UCAP_GameplayWidget::IsItemSwapMenuOpen()
{
	return MenuSwitcher && MenuSwitcher->GetVisibility() == ESlateVisibility::Visible && MenuSwitcher->GetActiveWidget() == ItemSwapWidget;
}

void UCAP_GameplayWidget::ActivateSwitcher()
{
	if (MenuSwitcher && CharacterMenuWidget)
	{
		MenuSwitcher->SetVisibility(ESlateVisibility::Visible);
		MenuSwitcher->SetActiveWidget(CharacterMenuWidget);
		CharacterMenuWidget->RefreshMenu();
	}
}

void UCAP_GameplayWidget::DeactivateSwitcher()
{
	if (MenuSwitcher)
	{
		MenuSwitcher->SetActiveWidget(CharacterMenuWidget);
		MenuSwitcher->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCAP_GameplayWidget::SwitchCharacterMenuTab()
{
	if (CharacterMenuWidget)
		CharacterMenuWidget->SwitchNextTab();
}

void UCAP_GameplayWidget::OpenItemSwapMenu(class UCAP_ItemInstance* NewItem)
{
	if (MenuSwitcher && ItemSwapWidget)
	{
		MenuSwitcher->SetVisibility(ESlateVisibility::Visible);
		MenuSwitcher->SetActiveWidget(ItemSwapWidget);
		
		ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
		ItemSwapWidget->InitSwapUI(Player, NewItem);
	}
}

void UCAP_GameplayWidget::HandleWeaponChanged(class UCAP_WeaponInstance* NewWeaponInstance)
{
	if (AbilityListView && NewWeaponInstance)
	{
		AbilityListView->RefreshWeaponSkills(NewWeaponInstance);
	}
}
