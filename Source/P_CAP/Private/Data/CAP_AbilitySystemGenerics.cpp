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

TSubclassOf<UGameplayEffect> UCAP_AbilitySystemGenerics::GetStatGE(bool bIsInfinite, bool bIsMultiplier) const
{
	if (bIsInfinite)
		return bIsMultiplier ? MasterStatInfiniteMulGE : MasterStatInfiniteAddGE;
	else
		return bIsMultiplier ? MasterStatDurationMulGE : MasterStatDurationAddGE;
}

FName UCAP_AbilitySystemGenerics::GetRowNameFromGrade(EItemGrade Grade)
{
	switch (Grade)
	{
	case EItemGrade::Normal:		return FName("Normal");
	case EItemGrade::Rare:			return FName("Rare");
	case EItemGrade::Epic:			return FName("Epic");
	case EItemGrade::Legendary:		return FName("Legendary");
	default:						return FName();
	}
}
