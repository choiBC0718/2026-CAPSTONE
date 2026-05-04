// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_GameplayWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAP_CharacterMenuWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/WidgetSwitcher.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Interface/CAP_MenuInterface.h"
#include "Widget/Common/CAP_ValueGauge.h"
#include "Widget/Common/CAP_WeaponSkillBox.h"
#include "Widget/PanelWidgets/CAP_BuffListPanelWidget.h"
#include "Widget/PanelWidgets/CAP_ItemSwapWidget.h"

void UCAP_GameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player)
		return;
	
	if (CharacterMenuWidget)
	{
		// 위젯 닫힘 델리게이트 연결
		CharacterMenuWidget->OnMenuClosed.AddDynamic(this, &UCAP_GameplayWidget::CompleteDeactivateSwitcher);
	}
	if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
	{
		InvComp->OnInventoryFull.AddDynamic(this, &UCAP_GameplayWidget::HandleInventoryFull);
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
	if (BuffListPanel)
	{
		BuffListPanel->InitializeWidget(Player);
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
		if (PickupItemDetailWidget)
			PickupItemDetailWidget->SetVisibility(ESlateVisibility::Collapsed);

		if (APlayerController* PC= GetOwningPlayer())
		{
			PC->SetPause(true);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
		}
	}
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

void UCAP_GameplayWidget::HandleInventoryFull(class UCAP_ItemInstance* NewItem)
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
		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->SetPause(true);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
		}
	}
}


void UCAP_GameplayWidget::RouteUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	if (IsItemSwapMenuOpen() && ItemSwapWidget)
	{
		if (TriggerEvent == ETriggerEvent::Triggered || TriggerEvent == ETriggerEvent::Completed)
		{
			ItemSwapWidget->ConfirmSwap();
		}
	}
	else if (IsCharacterMenuOpen() && CharacterMenuWidget)
	{
		CharacterMenuWidget->RouteUIConfirmInput(TriggerEvent, ElapsedTime);
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
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(false);
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		PC->SetInputMode(InputMode);
	}
	if (Player)
	{
		if (UCAP_InteractionComponent* InteractionComp = Player->GetInteractionComponent())
		{
			if (InteractionComp->GetNearbyInteractable()!=nullptr)
				if (PickupItemDetailWidget)
					PickupItemDetailWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

