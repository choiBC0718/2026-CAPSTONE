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

FGameplayTag UCAP_AbilitySystemStatics::GetTargetClearTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Attack.TargetClear");
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
