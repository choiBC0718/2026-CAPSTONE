// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting//CAP_AbilitySystemStatics.h"

FGameplayTag UCAP_AbilitySystemStatics::GetHealthFullStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Health.Full");
}

FGameplayTag UCAP_AbilitySystemStatics::GetHealthEmptyStatTag()
{
	return FGameplayTag::RequestGameplayTag("Stats.Health.Empty");
}

FGameplayTag UCAP_AbilitySystemStatics::GetDeadStateTag()
{
	return FGameplayTag::RequestGameplayTag("State.Dead");
}

FGameplayTag UCAP_AbilitySystemStatics::GetMovementBlockStateTag()
{
	return FGameplayTag::RequestGameplayTag("State.Block.Movement");
}

FGameplayTag UCAP_AbilitySystemStatics::GetBasicAttackTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Attack.Basic");
}

FGameplayTag UCAP_AbilitySystemStatics::GetSkillAttackTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Attack.Skill");
}

FGameplayTag UCAP_AbilitySystemStatics::GetDamageTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Event.Damage");
}

FGameplayTag UCAP_AbilitySystemStatics::GetRMSTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Event.RMS");
}

FGameplayTag UCAP_AbilitySystemStatics::GetSpawnProjectileTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Event.SpawnProjectile");
}

FGameplayTag UCAP_AbilitySystemStatics::GetAbilityChargeTimeTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Event.Charge.Time");
}

FGameplayTag UCAP_AbilitySystemStatics::GetDataDamageMultiplierTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.Multiplier");
}

FGameplayTag UCAP_AbilitySystemStatics::GetDataDamageBaseTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage.Base");
}

FGameplayTag UCAP_AbilitySystemStatics::GetDataDamageTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Damage");
}

FGameplayTag UCAP_AbilitySystemStatics::GetDataCooldownTag()
{
	return FGameplayTag::RequestGameplayTag("Data.Cooldown");
}

FGameplayTag UCAP_AbilitySystemStatics::GetDataEffectDurationTag()
{
	return FGameplayTag::RequestGameplayTag("Data.ItemEffect.Duration");
}

FGameplayTag UCAP_AbilitySystemStatics::GetDataStackTag()
{
	return FGameplayTag::RequestGameplayTag("Data.StackCount");
}

FGameplayTag UCAP_AbilitySystemStatics::GetItemTriggerCastBasic()
{
	return FGameplayTag::RequestGameplayTag("Item.Trigger.Cast.Basic");
}

FGameplayTag UCAP_AbilitySystemStatics::GetItemTriggerCastAbility()
{
	return FGameplayTag::RequestGameplayTag("Item.Trigger.Cast.Ability");
}

FGameplayTag UCAP_AbilitySystemStatics::GetItemTriggerHitBasic()
{
	return FGameplayTag::RequestGameplayTag("Item.Trigger.Hit.Basic");
}

FGameplayTag UCAP_AbilitySystemStatics::GetItemTriggerHitAbility()
{
	return FGameplayTag::RequestGameplayTag("Item.Trigger.Hit.Ability");
}