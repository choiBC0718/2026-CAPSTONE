// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_ItemGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CAP_AbilitySystemComponent.h"

UCAP_ItemGameplayAbility::UCAP_ItemGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UCAP_ItemGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ICAP_BehaviorStateProvider* StateProvider = Cast<ICAP_BehaviorStateProvider>(GetCurrentAbilitySpec()->SourceObject);
	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!StateProvider || !ASC)
	{
		K2_EndAbility();
		return;
	}

	for (UCAP_ItemBehaviorBase* Behavior : StateProvider->GetBehaviors())
	{
		if (Behavior)
			Behavior->OnEquipped(StateProvider, ASC);
	}
}

void UCAP_ItemGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ICAP_BehaviorStateProvider* StateProvider = Cast<ICAP_BehaviorStateProvider>(GetCurrentAbilitySpec()->SourceObject);
	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (StateProvider && ASC)
	{
		for (UCAP_ItemBehaviorBase* Behavior : StateProvider->GetBehaviors())
		{
			if (Behavior)
				Behavior->OnUnequipped(StateProvider, ASC);
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
