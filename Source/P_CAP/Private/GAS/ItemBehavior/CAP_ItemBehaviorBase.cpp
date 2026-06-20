// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"

#include "AbilitySystemComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"

UCAP_ItemBehaviorBase::UCAP_ItemBehaviorBase()
{
	BaseDamageTag = UCAP_AbilitySystemStatics::GetDataDamageBaseTag();
	DamageMultiplierTag = UCAP_AbilitySystemStatics::GetDataDamageMultiplierTag();
	StackTag = UCAP_AbilitySystemStatics::GetDataStackTag();
	DurationTag = UCAP_AbilitySystemStatics::GetDataEffectDurationTag();
}


void UCAP_ItemBehaviorBase::BindGameplayEvent(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC, FGameplayTag EventTag) const
{
	if (!StateProvider || !ASC || !EventTag.IsValid())
		return;
	/* ASC의 이벤트 시스템에 InternalEventCallback 함수 등록
	 * GenericGameplayEventCallbacks는 GAS에서 이벤트를 관리하는 Map
	 * EventTag를 Key로하는 Value(델리게이트)를 찾거나 새로 제작
	*/
	FDelegateHandle Handle = ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).AddUObject(this, &UCAP_ItemBehaviorBase::InternalEventCallback, StateProvider, ASC);
	/* 함수 등록 시 FDelegateHandle이라는 고유 번호 발금
	 * 이벤트 해제하게 되는 경우 이 번호 필요
	 */
	if (TArray<FDelegateHandle>* Handles = StateProvider->GetBoundEventHandles(this))
		Handles->Add(Handle);
}

void UCAP_ItemBehaviorBase::UnbindGameplayEvents(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	if (!StateProvider || !ASC)
		return;

	if (TArray<FDelegateHandle>* Handles = StateProvider->GetBoundEventHandles(this))
	{
		for (FDelegateHandle Handle : *Handles)
		{
			for (auto& Pair: ASC->GenericGameplayEventCallbacks)
			{
				Pair.Value.Remove(Handle);
			}
		}
		Handles->Empty();
	}
}

bool UCAP_ItemBehaviorBase::IsOnCooldown(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	if (Cooldown <= 0.f)
		return false;
	
	UWorld* World = ASC->GetAvatarActor()->GetWorld();
	if (!StateProvider || !ASC || !World)
		return false;

	float CurrentTime = World->GetTimeSeconds();
	float LastTimePtr = StateProvider->GetBehaviorLastTriggerTime(this);

	return (CurrentTime-LastTimePtr) < Cooldown;
}

void UCAP_ItemBehaviorBase::ConsumeCooldown(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	if (Cooldown<=0.f)
		return;
	UWorld* World = ASC->GetAvatarActor()->GetWorld();
	if (!StateProvider || !ASC || !World)
		return;

	StateProvider->SetBehaviorLastTriggerTime(this, World->GetTimeSeconds());
}

void UCAP_ItemBehaviorBase::InitGameplayEffectToDefault(const FGameplayEffectSpecHandle& SpecHandle,TSubclassOf<UGameplayEffect> BuffGE,float DefaultVal) const
{
	if (const UGameplayEffect* DefaultGE = BuffGE->GetDefaultObject<UGameplayEffect>())
	{
		for (const FGameplayModifierInfo& ModifierInfo : DefaultGE->Modifiers)
		{
			if (ModifierInfo.ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
			{
				FGameplayTag CallerTag= ModifierInfo.ModifierMagnitude.GetSetByCallerFloat().DataTag;
					SpecHandle.Data->SetSetByCallerMagnitude(CallerTag, DefaultVal);
			}
		}
	}
}

void UCAP_ItemBehaviorBase::InternalEventCallback(const struct FGameplayEventData* Payload,ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	OnEventReceived(StateProvider, ASC,Payload);
}
