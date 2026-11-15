// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_GameplayWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAP_CharacterMenuWidget.h"
#include "CAP_DialogueWidget.h"
#include "CAP_PauseMenuWidget.h"
#include "CAP_StatisticDashboardWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Character/Player/CAP_PlayerController.h"
#include "Components/WidgetSwitcher.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Interface/CAP_MenuInterface.h"
#include "Widget/Common/CAP_ValueGauge.h"
#include "Widget/Common/CAP_WeaponSkillBox.h"
#include "Widget/PanelWidgets/CAP_ItemSwapWidget.h"

void UCAP_GameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player)
		return;
	
	if (UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player))
	{
		if (HealthBar)
			HealthBar->SetAndBoundToGameplayAttribute(OwnerASC, UCAP_AttributeSet::GetHealthAttribute(), UCAP_AttributeSet::GetMaxHealthAttribute());
	}
	
	if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
	{
		InvComp->OnInventoryFull.AddDynamic(this, &UCAP_GameplayWidget::HandleInventoryFull);
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
		DialogueWidget->OnNPCCustomWidgetOpened.AddDynamic(this, &UCAP_GameplayWidget::HandleNPCCustomWidgetOpen);
	}
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

void UCAP_GameplayWidget::HandleDialogueFinished()
{
	CurrentActiveMenu = nullptr;

	if (ACAP_PlayerController* PC = GetOwningPlayer<ACAP_PlayerController>())
	{
		PC->SetUIInputEnabled(false);
	}

	if (Player)
	{
		if (UCAP_InteractionComponent* InteractionComp = Player->GetInteractionComponent())
		{
			if (InteractionComp->GetNearbyInteractable() != nullptr)
			{
				InteractPanelWidget->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}

void UCAP_GameplayWidget::HandleNPCCustomWidgetOpen(UUserWidget* TargetWidget)
{
	if (TargetWidget)
	{
		CurrentActiveMenu = TargetWidget;

		if (ACAP_PlayerController* PC = GetOwningPlayer<ACAP_PlayerController>())
		{
			PC->SetUIInputEnabled(true);
		}
	}
}

void UCAP_GameplayWidget::UINavigationHandle(FVector2D InputVal)
{
	if (CanProcessUINavigation())
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

void UCAP_GameplayWidget::ShowStatisticDashboard()
{
	if (StatDashboardWidget)
		ShowMenu(StatDashboardWidget);
}

void UCAP_GameplayWidget::HandleEscapeInput()
{
	// 1. 이미 메뉴(인벤토리, 스왑 창, 일시정지 등)가 열려 있으면 해당 메뉴 닫기
	if (CurrentActiveMenu && MenuSwitcher && MenuSwitcher->GetVisibility() == ESlateVisibility::Visible)
	{
		HideMenu();
		return;
	}

	// 2. NPC 대화창이 열려 있을 때는 ESC로 대화를 종료하지 않으며 일시정지 창도 겹쳐 띄우지 않음
	if (DialogueWidget && DialogueWidget->IsVisible())
	{
		return;
	}

	// 3. 인게임 상태일 때만 일시정지 메뉴 열기
	if (PauseMenuWidget)
	{
		ShowMenu(PauseMenuWidget);
	}
}

bool UCAP_GameplayWidget::CanProcessUINavigation()
{
	if (CurrentActiveMenu!=nullptr)
		return true;
	return false;
}

void UCAP_GameplayWidget::ToggleCharacterMenu()
{
	if (!MenuSwitcher || !CharacterMenuWidget) return;

	if (MenuSwitcher->GetVisibility() == ESlateVisibility::Collapsed)
	{
		if ((DialogueWidget && DialogueWidget->IsVisible()) || CurrentActiveMenu != nullptr)
		{
			return;
		}
		ShowMenu(CharacterMenuWidget);
	}
	else if (MenuSwitcher->GetVisibility() == ESlateVisibility::Visible && MenuSwitcher->GetActiveWidget() == CharacterMenuWidget)
	{
		CharacterMenuWidget->SwitchCharacterMenuTab();
	}
}

void UCAP_GameplayWidget::ShowMenu(UUserWidget* TargetMenuWidget)
{
	if (!TargetMenuWidget || CurrentActiveMenu == TargetMenuWidget) return;

	if (ICAP_MenuInterface* Interface = Cast<ICAP_MenuInterface>(TargetMenuWidget))
	{
		MenuSwitcher->SetVisibility(ESlateVisibility::Visible);
		MenuSwitcher->SetActiveWidget(TargetMenuWidget);
		CurrentActiveMenu = TargetMenuWidget;
		Interface->NativeOpenMenu();
		Interface->GetOnMenuClosedDelegate().AddUniqueDynamic(this, &UCAP_GameplayWidget::OnActiveMenuClosed);
		SetGamePause(true);
		InteractPanelWidget->SetVisibility(ESlateVisibility::Collapsed);

		if (ACAP_PlayerController* PC = GetOwningPlayer<ACAP_PlayerController>())
		{
			PC->SetUIInputEnabled(true);
		}
	}
}

void UCAP_GameplayWidget::HideMenu()
{
	if (CurrentActiveMenu && MenuSwitcher && MenuSwitcher->GetVisibility()==ESlateVisibility::Visible)
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
	}

	MenuSwitcher->SetVisibility(ESlateVisibility::Collapsed);
	CurrentActiveMenu = nullptr;
	SetGamePause(false);

	if (ACAP_PlayerController* PC = GetOwningPlayer<ACAP_PlayerController>())
	{
		PC->SetUIInputEnabled(false);
	}

	if (Player)
	{
		if (UCAP_InteractionComponent* InteractionComp = Player->GetInteractionComponent())
		{
			if (InteractionComp->GetNearbyInteractable() != nullptr)
			{
				InteractPanelWidget->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}

void UCAP_GameplayWidget::SetGamePause(bool Pause)
{
	if (APlayerController* PC = GetOwningPlayer())
		PC->SetPause(Pause);
}
