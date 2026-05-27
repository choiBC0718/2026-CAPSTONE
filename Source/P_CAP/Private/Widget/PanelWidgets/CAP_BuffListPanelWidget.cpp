// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_BuffListPanelWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/WrapBox.h"
#include "Component/CAP_InventoryComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Interactables/Item/CAP_ItemInstance.h"
#include "Widget/SlotWidgets/CAP_ItemEffectSlot.h"

void UCAP_BuffListPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuffWrapBox->ClearChildren();
	PassiveWrapBox->ClearChildren();

	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn()))
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player))
		{	// 스킬에서 발동한 버프를 위한 ASC 구독
			ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UCAP_BuffListPanelWidget::OnGEAdded);
			ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &UCAP_BuffListPanelWidget::OnGERemoved);
		}
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
		{	// 아이템에서 발동한 버프 인벤토리 구독
			InvComp->OnInventoryChanged.AddUniqueDynamic(this, &UCAP_BuffListPanelWidget::HandleInventoryChanged);
			InvComp->OnItemEffectTriggered.AddDynamic(this, &UCAP_BuffListPanelWidget::OnEffectTriggered);
		}
	}
}

void UCAP_BuffListPanelWidget::AddOrUpdateBuffSlot(const FBuffSlotID& SlotID, const FBuffUIData& UIData)
{
	UCAP_ItemEffectSlot* TargetSlot = FindSlot(SlotID);
	if (!TargetSlot)
	{
		TargetSlot = CreateWidget<UCAP_ItemEffectSlot>(this, EffectSlotClass);
		if (TargetSlot)
		{
			ActiveSlots.Add(TargetSlot);
		}
	}
	if (TargetSlot)
	{
		if (!TargetSlot->GetParent())
		{
			if (UIData.MaxDuration<=0.f && UIData.MaxCooldown<=0.f)
				PassiveWrapBox->AddChild(TargetSlot);
			else
				BuffWrapBox->AddChild(TargetSlot);
		}
		TargetSlot->InitSlot(SlotID, UIData);
	}
}

void UCAP_BuffListPanelWidget::RemoveBuffSlot(const FBuffSlotID& SlotID)
{
	for (int32 i=ActiveSlots.Num()-1; i>=0; --i)
	{
		UCAP_ItemEffectSlot* OldSlot = ActiveSlots[i];
		if (IsValid(OldSlot) && OldSlot->GetSlotID() == SlotID)
		{
			OldSlot->RemoveFromParent();
			ActiveSlots.RemoveAt(i);
			break;
		}
	}
}

class UCAP_ItemEffectSlot* UCAP_BuffListPanelWidget::FindSlot(const FBuffSlotID& SlotID) const
{
	for (UCAP_ItemEffectSlot* TargetSlot : ActiveSlots)
	{
		if (IsValid(TargetSlot) && TargetSlot->GetSlotID() == SlotID)
			return TargetSlot;
	}
	return nullptr;
}

void UCAP_BuffListPanelWidget::OnGEAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec,
	FActiveGameplayEffectHandle Handle)
{
	FGameplayTagContainer GrantedTags;
	Spec.GetAllGrantedTags(GrantedTags);
	// GE 스펙의 GrantedTag에 UI.Buff 또는 UI.Debuff가 있어야만 진행
	FGameplayTag BuffTag = FGameplayTag::RequestGameplayTag("UI.Buff");
	FGameplayTag DebuffTag = FGameplayTag::RequestGameplayTag("UI.Debuff");
	FGameplayTag StackTag = UCAP_AbilitySystemStatics::GetDataStackTag();
	
	if (!GrantedTags.HasTag(BuffTag) && !GrantedTags.HasTag(DebuffTag))
		return;

	UObject* SourceObj = Spec.GetContext().GetSourceObject();
	ICAP_BuffVisualInterface* VisualInterface = Cast<ICAP_BuffVisualInterface>(SourceObj);
	if (!VisualInterface)
		return;

	FGameplayTag IdentifierTag;
	for (const FGameplayTag& Tag : GrantedTags)
	{
		if (Tag.ToString().StartsWith(TEXT("Ability.Cooldown")))
		{
			IdentifierTag = Tag;
			break;
		}
	}
	FBuffDisplayData InterfaceData = VisualInterface->GetBuffDisplayData(IdentifierTag);
	float SetByCallerStack = Spec.GetSetByCallerMagnitude(StackTag, false, -1.f);
	
	FBuffSlotID SlotID;
	SlotID.SourceType = EBuffSourceType::ASC_Effect;
	SlotID.ActiveGEHandle = Handle;

	FBuffUIData UIData;
	UIData.Icon = InterfaceData.Icon;
	UIData.Stacks = FMath::RoundToInt(SetByCallerStack);
	UIData.MaxDuration = Spec.GetDuration();
	UIData.RemainingDuration = Spec.GetDuration();
	UIData.MaxCooldown = 0.f;
	UIData.RemainingCooldown = 0.f;
	UIData.bIsDebuff = GrantedTags.HasTag(DebuffTag);
	
	AddOrUpdateBuffSlot(SlotID, UIData);
}

void UCAP_BuffListPanelWidget::OnGERemoved(const FActiveGameplayEffect& EffectRemoved)
{
	FBuffSlotID SlotID;
	SlotID.SourceType = EBuffSourceType::ASC_Effect;
	SlotID.ActiveGEHandle = EffectRemoved.Handle;
	RemoveBuffSlot(SlotID);
}

void UCAP_BuffListPanelWidget::OnEffectTriggered(class UCAP_ItemInstance* ItemInst, FGameplayTag DynamicTag,
                                                 float Cooldown, float Duration, int32 Stacks)
{
	if (!ItemInst)
		return;
	
	FBuffSlotID SlotID;
	SlotID.SourceType = EBuffSourceType::Item_Effect;
	SlotID.ItemInst = ItemInst;
	SlotID.ItemDynamicTag = DynamicTag;

	FBuffUIData UIData;
	if (UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA())
	{
		UIData.Icon = ItemDA->ItemIcon;
	}
	UIData.Stacks = Stacks;
	UIData.MaxDuration = Duration;
	UIData.RemainingDuration = Duration;
	UIData.MaxCooldown = Cooldown;
	UIData.RemainingCooldown = Cooldown;
	UIData.bIsDebuff = false; // 아이템 특수효과는 보통 버프
	
	AddOrUpdateBuffSlot(SlotID, UIData);
}

void UCAP_BuffListPanelWidget::HandleInventoryChanged(class UCAP_ItemInstance* ChangedItem, bool bIsAdded)
{
	// 아이템 추가 시 무시, 아이템이 해제될 때만 발동
	if (!ChangedItem || bIsAdded)
		return;
	
	for (int32 i=ActiveSlots.Num()-1; i>=0; --i)
	{
		UCAP_ItemEffectSlot* TargetSlot = ActiveSlots[i];
		if (IsValid(TargetSlot))
		{
			FBuffSlotID ID = TargetSlot->GetSlotID();
			if (ID.SourceType == EBuffSourceType::Item_Effect && ID.ItemInst == ChangedItem)
			{
				TargetSlot->RemoveFromParent();
				ActiveSlots.RemoveAt(i);
			}
		}
	}
}