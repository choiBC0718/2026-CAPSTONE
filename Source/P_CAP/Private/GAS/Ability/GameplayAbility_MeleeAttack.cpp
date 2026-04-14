// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_MeleeAttack.h"

UGameplayAbility_MeleeAttack::UGameplayAbility_MeleeAttack()
{
}

void UGameplayAbility_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

}
