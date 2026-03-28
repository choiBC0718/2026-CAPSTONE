// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_MeleeAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGameplayAbility_MeleeAttack::UGameplayAbility_MeleeAttack()
{
}

void UGameplayAbility_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AbilityMontage);
	MontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_MeleeAttack::K2_EndAbility);
	MontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_MeleeAttack::K2_EndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_MeleeAttack::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_MeleeAttack::K2_EndAbility);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* DamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, DamageTag);
	DamageTask->EventReceived.AddDynamic(this, &UGameplayAbility_MeleeAttack::OnDamageTagReceived);
	DamageTask->ReadyForActivation();
}

void UGameplayAbility_MeleeAttack::OnDamageTagReceived(FGameplayEventData Payload)
{
	int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(Payload.TargetData);

	UE_LOG(LogTemp, Warning, TEXT("데미지 태그 발동"));

	for (int i = 0; i < HitResultCount; i++)
	{
		FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Payload.TargetData, i);

		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()));
		FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
		EffectContext.AddHitResult(HitResult);

		EffectSpecHandle.Data->SetContext(EffectContext);

		ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
	}
}
