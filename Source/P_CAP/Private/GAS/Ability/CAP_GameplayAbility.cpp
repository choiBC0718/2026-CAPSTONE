// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


void UCAP_GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* PlayMontageAndWait = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
	if (PlayMontageAndWait)
	{
		PlayMontageAndWait->OnCompleted.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
		PlayMontageAndWait->OnCancelled.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
		PlayMontageAndWait->OnBlendOut.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
		PlayMontageAndWait->OnInterrupted.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
		PlayMontageAndWait->ReadyForActivation();
	}
}

void UCAP_GameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
