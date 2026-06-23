// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_AreaOfEffect.h"

void UItemBehavior_AreaOfEffect::OnEquipped(ICAP_BehaviorStateProvider* StateProvider,
	UCAP_AbilitySystemComponent* ASC) const
{
	BindGameplayEvent(StateProvider,ASC,TriggerEventTag);
}

void UItemBehavior_AreaOfEffect::OnUnequipped(ICAP_BehaviorStateProvider* StateProvider,
	UCAP_AbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(StateProvider,ASC);
}

void UItemBehavior_AreaOfEffect::OnEventReceived(ICAP_BehaviorStateProvider* StateProvider,
	UCAP_AbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const
{

}
