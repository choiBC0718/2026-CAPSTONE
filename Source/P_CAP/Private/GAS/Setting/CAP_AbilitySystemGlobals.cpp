// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting/CAP_AbilitySystemGlobals.h"

#include "CAP_GameplayAbilityTypes.h"

FGameplayEffectContext* UCAP_AbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FCAP_GameplayEffectContext();
}
