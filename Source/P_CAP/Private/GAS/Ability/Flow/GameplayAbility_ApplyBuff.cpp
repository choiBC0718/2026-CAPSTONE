// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Flow/GameplayAbility_ApplyBuff.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void UGameplayAbility_ApplyBuff::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                 const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
/*
	UAbilityTask_WaitGameplayEvent* WaitBuffTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent();
	WaitBuffTask->EventReceived.AddDynamic(this, &UGameplayAbility_ApplyBuff::OnBuffTagReceived);
	WaitBuffTask->ReadyForActivation();*/
}

void UGameplayAbility_ApplyBuff::OnBuffTagReceived(FGameplayEventData Payload)
{
}
