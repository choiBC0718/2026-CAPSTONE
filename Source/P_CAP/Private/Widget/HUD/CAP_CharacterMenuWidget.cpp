// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_CharacterMenuWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/Border.h"
#include "Widget/PanelWidgets/CAP_InventoryTabWidget.h"
#include "Widget/PanelWidgets/CAP_ItemEquipPanelWidget.h"

void UCAP_CharacterMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>())
	{
		if (UCAP_InventoryComponent* InventoryComp = Player->GetInventoryComponent())
			InventoryComp->OnInventoryChanged.AddDynamic(this, &UCAP_CharacterMenuWidget::HandleInventoryChanged);
	}
}

void UCAP_CharacterMenuWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);
	if (Animation == CloseAnim)
	{
		OnMenuClosed.Broadcast();
	}
}

void UCAP_CharacterMenuWidget::NativeOpenMenu()
{
	bIsAttributeTabOpen = false;
	if (SwitchTab)
	{
		StopAnimation(SwitchTab);
	}
	if (InventoryBorder)
	{
		InventoryBorder->SetRenderOpacity(1.f);
		InventoryBorder->SetVisibility(ESlateVisibility::Visible);
	}
	if (SlideAnim)
	{
		PlayAnimation(SlideAnim, 0.f, 1, EUMGSequencePlayMode::Forward,1.f,true);
	}
	RefreshMenu();
}

void UCAP_CharacterMenuWidget::NativeCloseMenu()
{
	if (CloseAnim)
	{
		PlayAnimation(CloseAnim, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f, true);
	}
	else
	{
		OnMenuClosed.Broadcast();
	}
}

FOnMenuClosedSignature& UCAP_CharacterMenuWidget::GetOnMenuClosedDelegate()
{
	return OnMenuClosed;
}

void UCAP_CharacterMenuWidget::NavigationInput(FVector2D InputVal)
{
	if (InventoryTabWidget && InventoryTabWidget->IsVisible())
	{
		InventoryTabWidget->NavigationInput(InputVal);
	}
}

void UCAP_CharacterMenuWidget::RefreshMenu()
{
	if (InventoryTabWidget)
	{
		InventoryTabWidget->SetVisibility(ESlateVisibility::Visible);
		InventoryTabWidget->SetRenderOpacity(1.f);
		if (ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn()))
		{
			InventoryTabWidget->RefreshInventoryTab(PlayerCharacter);
		}
	}
}

void UCAP_CharacterMenuWidget::SwitchCharacterMenuTab()
{
	if (!bIsAttributeTabOpen)
	{
		bIsAttributeTabOpen = true;
		PlayAnimation(SwitchTab, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f, false);
	}else
	{
		bIsAttributeTabOpen = false;
		PlayAnimation(SwitchTab, 0.f, 1, EUMGSequencePlayMode::Reverse, 1.f, false);
	}
}

void UCAP_CharacterMenuWidget::RouteUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	if (!bIsAttributeTabOpen && InventoryTabWidget && InventoryTabWidget->GetItemEquipPanel())
	{
		InventoryTabWidget->GetItemEquipPanel()->HandleInteractionInput(TriggerEvent, ElapsedTime);
	}
}

FReply UCAP_CharacterMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	return FReply::Handled();
}

void UCAP_CharacterMenuWidget::HandleInventoryChanged(class UCAP_ItemInstance* ChangedItem, bool bIsAdded)
{
	RefreshMenu();
}
