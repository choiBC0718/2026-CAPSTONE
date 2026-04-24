// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_GameplayWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAP_CharacterMenuWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/WidgetSwitcher.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Interface/CAP_MenuInterface.h"
#include "Items/Weapon/CAP_WeaponComponent.h"
#include "Widget/Common/CAP_ValueGauge.h"
#include "Widget/Common/CAP_AbilityListView.h"
#include "Widget/PanelWidgets/CAP_InventoryTabWidget.h"
#include "Widget/PanelWidgets/CAP_ItemEquipPanelWidget.h"
#include "Widget/PanelWidgets/CAP_ItemSwapWidget.h"

void UCAP_GameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
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
	if (PickupItemDetailWidget)
	{
		PickupItemDetailWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (MenuSwitcher)
	{
		MenuSwitcher->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (CharacterMenuWidget)
	{
		// 위젯 닫힘 델리게이트 연결
		CharacterMenuWidget->OnMenuClosed.AddDynamic(this, &UCAP_GameplayWidget::CompleteDeactivateSwitcher);
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

// WidgetSwitcher 활성화, UI 오픈 애니메이션 재생
void UCAP_GameplayWidget::ActivateSwitcher()
{
	if (MenuSwitcher && CharacterMenuWidget)
	{
		MenuSwitcher->SetVisibility(ESlateVisibility::Visible);
		MenuSwitcher->SetActiveWidget(CharacterMenuWidget);
		if (ICAP_MenuInterface* Menu = Cast<ICAP_MenuInterface>(CharacterMenuWidget))
		{
			Menu->NativeOpenMenu();
		}
	}
	if (PickupItemDetailWidget)
		PickupItemDetailWidget->SetVisibility(ESlateVisibility::Collapsed);
}
// WidgetSwitcher 비활성화, UI 닫히는 애니메이션 재생
void UCAP_GameplayWidget::DeactivateSwitcher()
{
	if (!MenuSwitcher)
		return;

	UUserWidget* ActiveWidget = Cast<UUserWidget>(MenuSwitcher->GetActiveWidget());
	ICAP_MenuInterface* Menu = Cast<ICAP_MenuInterface>(ActiveWidget);
	if (Menu)
	{
		Menu->GetOnMenuClosedDelegate().AddUniqueDynamic(this, &UCAP_GameplayWidget::CompleteDeactivateSwitcher);
		Menu->NativeCloseMenu();
	}
	else
	{
		CompleteDeactivateSwitcher();
	}
}
// InventoryTab <-> AttributeTab 
void UCAP_GameplayWidget::SwitchCharacterMenuTab()
{
	if (CharacterMenuWidget)
		CharacterMenuWidget->SwitchCharacterMenuTab();
}

void UCAP_GameplayWidget::OpenItemSwapMenu(class UCAP_ItemInstance* NewItem)
{
	if (PickupItemDetailWidget && MenuSwitcher && ItemSwapWidget && Player)
	{
		PickupItemDetailWidget->SetVisibility(ESlateVisibility::Collapsed);
		
		MenuSwitcher->SetVisibility(ESlateVisibility::Visible);
		MenuSwitcher->SetActiveWidget(ItemSwapWidget);
		ItemSwapWidget->InitSwapUI(Player, NewItem);
		if (ICAP_MenuInterface* Menu = Cast<ICAP_MenuInterface>(ItemSwapWidget))
		{
			Menu->NativeOpenMenu();
		}
	}
}

// 아이템 상호작용 게이지 업데이트
void UCAP_GameplayWidget::UpdateInteractProgress(float Progress)
{
	if (PickupItemDetailWidget)
	{
		PickupItemDetailWidget->UpdateInteractProgress(Progress);
	}
}

// 아이템 상호작용 위젯 정보 업데이트
void UCAP_GameplayWidget::UpdateInteractionUI(bool bVisible, UObject* ItemData, const FString& KeyName)
{
	if (PickupItemDetailWidget)
	{
		PickupItemDetailWidget->UpdateInteractionUI(bVisible, ItemData, KeyName);
	}
}

// 위젯 닫히는 애니메이션 끝나면 실행됨
void UCAP_GameplayWidget::CompleteDeactivateSwitcher()
{
	if (MenuSwitcher)
	{
		UUserWidget* ActiveWidget = Cast<UUserWidget>(MenuSwitcher->GetActiveWidget());
		if (ICAP_MenuInterface* Menu = Cast<ICAP_MenuInterface>(ActiveWidget))
		{
			Menu->GetOnMenuClosedDelegate().RemoveDynamic(this, &UCAP_GameplayWidget::CompleteDeactivateSwitcher);
		}
		MenuSwitcher->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
	{
		if (InvComp->GetNearbyInteractable()!=nullptr)
			if (PickupItemDetailWidget)
				PickupItemDetailWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

// 스킬 ListView 아이콘 변경
void UCAP_GameplayWidget::HandleWeaponChanged(class UCAP_WeaponInstance* NewWeaponInstance)
{
	if (AbilityListView && NewWeaponInstance)
	{
		AbilityListView->RefreshWeaponSkills(NewWeaponInstance);
	}
}
