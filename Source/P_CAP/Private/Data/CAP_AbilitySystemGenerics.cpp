// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/CAP_AbilitySystemGenerics.h"

TSubclassOf<UGameplayEffect> UCAP_AbilitySystemGenerics::GetInstantDamageGE(ESkillDamageType Type) const
{
	switch (Type)
	{
		case ESkillDamageType::Physical:
			return MasterPhysicalInstantDamageGE;
		case ESkillDamageType::Magical:
			return MasterMagicalInstantDamageGE;
		default:
			return nullptr;
	}
}

TSubclassOf<UGameplayEffect> UCAP_AbilitySystemGenerics::GetDurationDamageGE(ESkillDamageType Type) const
{
	switch (Type)
	{
		case ESkillDamageType::Physical:
			return MasterPhysicalDurationDamageGE;
		case ESkillDamageType::Magical:
			return MasterMagicalDurationDamageGE;
		default:
			return nullptr;
	}
}
