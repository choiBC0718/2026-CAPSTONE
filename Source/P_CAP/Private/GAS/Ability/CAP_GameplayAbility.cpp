// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_GameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"


UCAP_GameplayAbility::UCAP_GameplayAbility()
{
	DamageTag = UCAP_AbilitySystemStatics::GetDamageTag();
}

bool UCAP_GameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (AbilitySpec && AbilitySpec->Level <= 0)
		return false;
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UCAP_GameplayAbility::ApplyGameplayEffectToHitResult(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, Level);
	FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	EffectContext.AddHitResult(HitResult);
	EffectSpecHandle.Data->SetContext(EffectContext);
	
	ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
}
