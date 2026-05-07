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
	
	if (CharacterMenuWidget)
	{
		// 위젯 닫힘 델리게이트 연결
		CharacterMenuWidget->OnMenuClosed.AddDynamic(this, &UCAP_GameplayWidget::CompleteDeactivateSwitcher);
	}
	if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
	{
		InvComp->OnInventoryFull.AddDynamic(this, &UCAP_GameplayWidget::HandleInventoryFull);
	}
	if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
	{
		InteractComp->OnDialogueTriggered.AddDynamic(this, &UCAP_GameplayWidget::HandleDialogueTriggered);
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
	if (CharacterMenuWidget)
		ShowMenuWidget(CharacterMenuWidget);
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
	if (ItemSwapWidget && Player)
	{
		ShowMenuWidget(ItemSwapWidget);
		ItemSwapWidget->InitSwapUI(Player, NewItem);
	}
}

void UCAP_GameplayWidget::HandleDialogueTriggered(const FNPCData& NPCData)
{
	if (!DialogueWidget)
		return;

	DialogueWidget->SetVisibility(ESlateVisibility::Visible);
	DialogueWidget->StartDialogue();
	DialogueWidget->UpdateDialogueUI(NPCData);
	InteractPanelWidget->SetVisibility(ESlateVisibility::Collapsed);

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(DialogueWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
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

void UCAP_GameplayWidget::ShowMenuWidget(UUserWidget* TargetMenuWidget)
{
	if (!MenuSwitcher || !TargetMenuWidget)
		return;
	
	if (InteractPanelWidget)
		InteractPanelWidget->SetVisibility(ESlateVisibility::Collapsed);

	MenuSwitcher->SetVisibility(ESlateVisibility::Visible);
	MenuSwitcher->SetActiveWidget(TargetMenuWidget);
	if (ICAP_MenuInterface* Menu = Cast<ICAP_MenuInterface>(TargetMenuWidget))
		Menu->NativeOpenMenu();

	EnterUIMode();
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
	if (Player)
	{
		if (UCAP_InteractionComponent* InteractionComp = Player->GetInteractionComponent())
		{
			if (InteractionComp->GetNearbyInteractable()!=nullptr)
				if (InteractPanelWidget)
					InteractPanelWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
	ExitUIMode();
}

void UCAP_GameplayWidget::HandleDialogueFinished()
{
	InteractPanelWidget->SetVisibility(ESlateVisibility::Visible);
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}

void UCAP_GameplayWidget::EnterUIMode()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}
}

void UCAP_GameplayWidget::ExitUIMode()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(false);
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		PC->SetInputMode(InputMode);
	}
}
