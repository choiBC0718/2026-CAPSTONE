// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_GameplayWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAP_CharacterMenuWidget.h"
#include "CAP_DialogueWidget.h"
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
	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);
	if (!OwnerASC)
		return;
	
	if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
	{
		InvComp->OnInventoryFull.AddDynamic(this, &UCAP_GameplayWidget::HandleInventoryFull);
	}
	if (HealthBar)
	{
		HealthBar->SetAndBoundToGameplayAttribute(OwnerASC, UCAP_AttributeSet::GetHealthAttribute(), UCAP_AttributeSet::GetMaxHealthAttribute());
	}
	if (InteractPanelWidget)
	{
		InteractPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (MenuSwitcher)
	{
		MenuSwitcher->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (DialogueWidget)
	{
		DialogueWidget->SetVisibility(ESlateVisibility::Collapsed);
		DialogueWidget->OnDialogueFinished.AddDynamic(this, &UCAP_GameplayWidget::HandleDialogueFinished);
	}
	if (BuffListPanel)
	{
		BuffListPanel->InitializeWidget(Player);
	}
}

// WidgetSwitcher 활성화, UI 오픈 애니메이션 재생
void UCAP_GameplayWidget::ActivateSwitcher()
{
	if (CharacterMenuWidget)
	{
		ShowMenu(CharacterMenuWidget);
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
	if (ItemSwapWidget && Player)
	{
		ShowMenu(ItemSwapWidget);
		if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
			ItemSwapWidget->InitSwapUI(Player, InteractComp->LastInventoryFullActor,NewItem);
	}
}

void UCAP_GameplayWidget::UINavigationHandle(FVector2D InputVal)
{
	if (CurrentActiveMenu)
	{
		if (ICAP_MenuInterface* Interface = Cast<ICAP_MenuInterface>(CurrentActiveMenu))
		{
			Interface->HandleChangeSelectedSlot(InputVal);
		}
	}
}

void UCAP_GameplayWidget::RouteUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	if (CurrentActiveMenu)
	{
		if (ICAP_MenuInterface* Interface = Cast<ICAP_MenuInterface>(CurrentActiveMenu))
		{
			Interface->HandleUIConfirmInput(TriggerEvent, ElapsedTime);
		}
	}
}

void UCAP_GameplayWidget::HandleDialogueFinished()
{
	if (UCAP_InteractionComponent* InteractionComp = Player->GetInteractionComponent())
	{
		if (InteractionComp->GetNearbyInteractable() != nullptr)
			InteractPanelWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCAP_GameplayWidget::EnterUIMode()
{
	if (APlayerController* PC = GetOwningPlayer())
		PC->SetPause(true);
}

void UCAP_GameplayWidget::ExitUIMode()
{
	if (APlayerController* PC = GetOwningPlayer())
		PC->SetPause(false);
}

bool UCAP_GameplayWidget::IsCharacterMenuOpen()
{
	return MenuSwitcher && MenuSwitcher->GetVisibility() == ESlateVisibility::Visible && MenuSwitcher->GetActiveWidget() == CharacterMenuWidget;
}

bool UCAP_GameplayWidget::IsItemSwapMenuOpen()
{
	return MenuSwitcher && MenuSwitcher->GetVisibility() == ESlateVisibility::Visible && MenuSwitcher->GetActiveWidget() == ItemSwapWidget;
}

bool UCAP_GameplayWidget::IsMenuSwitcherVisible()
{
	return MenuSwitcher->GetVisibility() == ESlateVisibility::Visible;
}

bool UCAP_GameplayWidget::IsDialogueWidgetOpen()
{
	return DialogueWidget && DialogueWidget->GetVisibility() == ESlateVisibility::Visible;
}

void UCAP_GameplayWidget::ShowMenu(UUserWidget* TargetMenuWidget)
{
	if (!TargetMenuWidget || CurrentActiveMenu==TargetMenuWidget)
		return;

	if (ICAP_MenuInterface* Interface = Cast<ICAP_MenuInterface>(TargetMenuWidget))
	{
		MenuSwitcher->SetVisibility(ESlateVisibility::Visible);
		MenuSwitcher->SetActiveWidget(TargetMenuWidget);
		
		CurrentActiveMenu = TargetMenuWidget;
		
		Interface->NativeOpenMenu();
		Interface->GetOnMenuClosedDelegate().AddUniqueDynamic(this, &UCAP_GameplayWidget::OnActiveMenuClosed);
	}
	EnterUIMode();
	
	InteractPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UCAP_GameplayWidget::HideMenu()
{
	if (CurrentActiveMenu)
	{
		if (ICAP_MenuInterface* Interface = Cast<ICAP_MenuInterface>(CurrentActiveMenu))
		{
			Interface->NativeCloseMenu();
		}
	}
}

void UCAP_GameplayWidget::OnActiveMenuClosed()
{
	if (CurrentActiveMenu)
	{
		if (ICAP_MenuInterface* Interface = Cast<ICAP_MenuInterface>(CurrentActiveMenu))
		{
			Interface->GetOnMenuClosedDelegate().RemoveDynamic(this, &UCAP_GameplayWidget::OnActiveMenuClosed);
		}
		MenuSwitcher->SetVisibility(ESlateVisibility::Collapsed);
		CurrentActiveMenu = nullptr;
		ExitUIMode();
	}
	if (UCAP_InteractionComponent* InteractionComp = Player->GetInteractionComponent())
	{
		if (InteractionComp->GetNearbyInteractable() != nullptr)
			InteractPanelWidget->SetVisibility(ESlateVisibility::Visible);
	}
}
