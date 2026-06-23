// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_ConditionalPassive.h"

#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Ability/Item/GA_PassiveItemMonitor.h"

void UItemBehavior_ConditionalPassive::OnEquipped(ICAP_BehaviorStateProvider* StateProvider,
                                                  UCAP_AbilitySystemComponent* ASC) const
{
	if (!ASC || !MonitorAbilityClass)
		return;

	FGameplayAbilitySpec Spec(MonitorAbilityClass, 1, INDEX_NONE, const_cast<UItemBehavior_ConditionalPassive*>(this));
	FGameplayAbilitySpecHandle GrantedHandle = ASC->GiveAbilityAndActivateOnce(Spec);

	if (TArray<FGameplayAbilitySpecHandle>* Handles = StateProvider->GetGrantedAbilityHandles(this))
	{
		Handles->Add(GrantedHandle);
	}
}

void UItemBehavior_ConditionalPassive::OnUnequipped(ICAP_BehaviorStateProvider* StateProvider,
	UCAP_AbilitySystemComponent* ASC) const
{
	if (!ASC || !StateProvider)
		return;

	// 저장해둔 핸들을 모두 꺼내서 패시브 어빌리티 회수 (버프 제거 및 감시 종료)
	if (TArray<FGameplayAbilitySpecHandle>* Handles = StateProvider->GetGrantedAbilityHandles(this))
	{
		for (const FGameplayAbilitySpecHandle& Handle : *Handles)
		{
			ASC->ClearAbility(Handle);
		}
		Handles->Empty();
	}
}
