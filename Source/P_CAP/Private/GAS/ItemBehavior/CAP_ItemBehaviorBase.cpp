// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"

#include "AbilitySystemComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Interactables/Item/CAP_ItemInstance.h"

UCAP_ItemBehaviorBase::UCAP_ItemBehaviorBase()
{
	BaseDamageTag = UCAP_AbilitySystemStatics::GetDataDamageBaseTag();
	DamageMultiplierTag = UCAP_AbilitySystemStatics::GetDataDamageMultiplierTag();
	StackTag = UCAP_AbilitySystemStatics::GetDataStackTag();
	DurationTag = UCAP_AbilitySystemStatics::GetDataEffectDurationTag();
}

void UCAP_ItemBehaviorBase::OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	if (ItemInst && ASC)
	{
		if (UCAP_AbilitySystemComponent* CAP_ASC = Cast<UCAP_AbilitySystemComponent>(ASC))
			ItemInst->SetCachedASC(CAP_ASC);
	}
}

void UCAP_ItemBehaviorBase::OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	if (ItemInst)
		ItemInst->SetCachedASC(nullptr);
}

void UCAP_ItemBehaviorBase::BindGameplayEvent(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, FGameplayTag EventTag) const
{
	if (!ItemInst || !ASC || !EventTag.IsValid())
		return;
	/* ASC의 이벤트 시스템에 InternalEventCallback 함수 등록
	 * GenericGameplayEventCallbacks는 GAS에서 이벤트를 관리하는 Map
	 * EventTag를 Key로하는 Value(델리게이트)를 찾거나 새로 제작
	*/
	FDelegateHandle Handle = ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).AddUObject(this, &UCAP_ItemBehaviorBase::InternalEventCallback, ItemInst, ASC);
	/* 함수 등록 시 FDelegateHandle이라는 고유 번호 발금
	 * 이벤트 해제하게 되는 경우 이 번호 필요
	 * Behavior클래스는 ItemDA에 들어가므로 메모리 공유 => 아이템마다 개인의 변수로 관리하기 위해 ItemInst에 번호 저장
	 */
	ItemInst->BoundEventHandles.FindOrAdd(this).Add(Handle);
}

void UCAP_ItemBehaviorBase::UnbindGameplayEvents(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	if (!ItemInst || !ASC)
		return;
	// ItemInst에 저장해놓은 고유 번호로 찾아 이벤트 제거
	if (TArray<FDelegateHandle>* Handles = ItemInst->BoundEventHandles.Find(this))
	{
		for (FDelegateHandle Handle : *Handles)
		{
			for (auto& Pair: ASC->GenericGameplayEventCallbacks)
			{
				Pair.Value.Remove(Handle);
			}
		}
		ItemInst->BoundEventHandles.Remove(this);
	}
}

bool UCAP_ItemBehaviorBase::IsOnCooldown(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	if (Cooldown <= 0.f)
		return false;
	
	UWorld* World = ASC->GetAvatarActor()->GetWorld();
	if (!ItemInst || !ASC || !World)
		return false;

	float CurrentTime = World->GetTimeSeconds();
	const float* LastTimePtr = ItemInst->BehaviorLastTriggerTimes.Find(this);
	float LastTriggerTime = LastTimePtr ? *LastTimePtr : -999.f;

	return (CurrentTime - LastTriggerTime < Cooldown);
}

void UCAP_ItemBehaviorBase::ConsumeCooldown(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	if (Cooldown<=0.f)
		return;
	UWorld* World = ASC->GetAvatarActor()->GetWorld();
	if (!ItemInst || !ASC || !World)
		return;

	ItemInst->BehaviorLastTriggerTimes.FindOrAdd(this) = World->GetTimeSeconds();
}

void UCAP_ItemBehaviorBase::InitGameplayEffectToZero(const FGameplayEffectSpecHandle& SpecHandle,TSubclassOf<UGameplayEffect> BuffGE) const
{
	if (const UGameplayEffect* DefaultGE = BuffGE->GetDefaultObject<UGameplayEffect>())
	{
		for (const FGameplayModifierInfo& ModifierInfo : DefaultGE->Modifiers)
		{
			if (ModifierInfo.ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
			{
				FGameplayTag CallerTag= ModifierInfo.ModifierMagnitude.GetSetByCallerFloat().DataTag;
					SpecHandle.Data->SetSetByCallerMagnitude(CallerTag, 0.f);
			}
		}
	}
}

void UCAP_ItemBehaviorBase::InternalEventCallback(const struct FGameplayEventData* Payload, UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	OnEventReceived(ItemInst, ASC,Payload);
}
