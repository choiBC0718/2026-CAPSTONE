// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/CAP_AbilitySystemGenerics.h"

TSubclassOf<UGameplayEffect> UCAP_AbilitySystemGenerics::GetDamageGE(ESkillDamageType Type) const
{
	switch (Type)
	{
		case ESkillDamageType::Physical:
			return MasterPhysicalDamageGE;
		case ESkillDamageType::Magical:
			return MasterMagicalDamageGE;
		default:
			return nullptr;
	}
}

TSubclassOf<UGameplayEffect> UCAP_AbilitySystemGenerics::GetItemMasterGE(EItemExecutionType Type) const
{
	switch (Type)
	{
	case EItemExecutionType::Instant_Damage:
		return MasterInstantDamageGE;
	case EItemExecutionType::DotDamage:
		return MasterDotDamageGE;
	case EItemExecutionType::Buff_Self:
	case EItemExecutionType::Debuff_Target:
		return MasterStatDurationGE;
	default:
			return nullptr;
	}
}
