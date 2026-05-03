// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_BuffListPanelWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/WrapBox.h"
#include "Items/Item/CAP_InventoryComponent.h"
#include "Items/Item/CAP_ItemInstance.h"
#include "Widget/SlotWidgets/CAP_ItemEffectSlot.h"

void UCAP_BuffListPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuffWrapBox->ClearChildren();
	PassiveWrapBox->ClearChildren();
}

void UCAP_BuffListPanelWidget::InitializeWidget(class ACAP_PlayerCharacter* InPlayerCharacter)
{
	if (!InPlayerCharacter)
		return;
	if (UCAP_InventoryComponent* InvComp = InPlayerCharacter->GetInventoryComponent())
	{
		InvComp->OnInventoryChanged.AddUniqueDynamic(this, &UCAP_BuffListPanelWidget::HandleInventoryChanged);
		InvComp->OnItemEffectTriggered.AddDynamic(this, &UCAP_BuffListPanelWidget::OnEffectTriggered);
	}
}

void UCAP_BuffListPanelWidget::OnEffectTriggered(class UCAP_ItemInstance* ItemInst, FGameplayTag DynamicTag,
	float Cooldown, float Duration, int32 Stacks)
{
	if (!ItemInst || !EffectSlotClass)
		return;

	CleanUpInvalidSlots();
	UCAP_ItemEffectSlot* TargetSlot = FindExistingSlot(ItemInst, DynamicTag);
	if (!TargetSlot)
	{
		TargetSlot = CreateWidget<UCAP_ItemEffectSlot>(this, EffectSlotClass);
		if (TargetSlot)
		{
			ActiveSlots.Add(TargetSlot);
			if (Duration<=0.f)
				PassiveWrapBox->AddChild(TargetSlot);
			else
				BuffWrapBox->AddChild(TargetSlot);
		}
	}
	if (TargetSlot)
	{
		TargetSlot->InitSlot(ItemInst, DynamicTag, Cooldown, Duration, Stacks);
	}
}

void UCAP_BuffListPanelWidget::HandleInventoryChanged(class UCAP_ItemInstance* ChangedItem, bool bIsAdded)
{
	// 아이템 추가 시 무시, 아이템이 해제될 때만 발동
	if (!ChangedItem)
		return;
	CleanUpInvalidSlots();

	if (!bIsAdded)
	{
		for (int32 i=ActiveSlots.Num()-1; i>=0; --i)
		{
			UCAP_ItemEffectSlot* EffectSlot = ActiveSlots[i];
			if (EffectSlot && EffectSlot->GetItemInstance() == ChangedItem)
			{
				EffectSlot->RemoveFromParent();
				ActiveSlots.RemoveAt(i);
			}
		}
	}
}

class UCAP_ItemEffectSlot* UCAP_BuffListPanelWidget::FindExistingSlot(class UCAP_ItemInstance* ItemInst,FGameplayTag DynamicTag) const
{
	for (UCAP_ItemEffectSlot* EffectSlot : ActiveSlots)
	{
		if (IsValid(EffectSlot) && EffectSlot->GetItemInstance() == ItemInst && EffectSlot->GetDynamicTag() == DynamicTag)
			return EffectSlot;
	}
	return nullptr;
}

void UCAP_BuffListPanelWidget::CleanUpInvalidSlots()
{
	ActiveSlots.RemoveAll([](UCAP_ItemEffectSlot* EffectSlot)
	{
		return !IsValid(EffectSlot) || !EffectSlot->GetParent();
	});
}
