// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Payload/GA_Payload_InstantDamage.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"

UGA_Payload_InstantDamage::UGA_Payload_InstantDamage()
{
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = UCAP_AbilitySystemStatics::GetDamageTag();
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_Payload_InstantDamage::ExecutePayloadLogic(const FGameplayEventData& EventData)
{
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	if (!SkillData)
		return;

	int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(EventData.TargetData);
	TSubclassOf<UGameplayEffect> DamageGE = GetDamageGE();
	if (!DamageGE)
		return;
	
	for (int i=0 ; i<HitResultCount ; i++)
	{
		FHitResult Hit = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, i);
		FGameplayEffectContextHandle Context = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
		Context.AddHitResult(Hit);
		
		FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGE, GetAbilityLevel());
		if (DamageSpecHandle.IsValid())
		{
			DamageSpecHandle.Data->SetContext(Context);
			DamageSpecHandle.Data->SetSetByCallerMagnitude(BaseDamageDataTag, SkillData->BaseDamage);
			DamageSpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierDataTag, SkillData->DamageMultiplier);
			DamageSpecHandle.Data->SetSetByCallerMagnitude(ChargeMultiplierDataTag, EventData.EventMagnitude);

			ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, DamageSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Hit.GetActor()));
		}
		SendGameplayCueEvent(Hit, SkillData);
	}

	if (HitResultCount > 0)
	{
		FGameplayTag ResultHitTag= IsBasicAttack()?TriggerHitBasicTag : TriggerHitAbilityTag;
		BroadcastTriggerEvent(ResultHitTag, EventData.TargetData);
	}
}
