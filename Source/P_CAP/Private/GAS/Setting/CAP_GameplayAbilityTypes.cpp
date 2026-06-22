// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting/CAP_GameplayAbilityTypes.h"

bool FCAP_GameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Super::NetSerialize(Ar, Map, bOutSuccess);
	Ar << bIsCritical;
	bOutSuccess = true;
	return true;
}

FCAP_GameplayEffectContext* FCAP_GameplayEffectContext::Duplicate() const
{
	FCAP_GameplayEffectContext* NewContext = new FCAP_GameplayEffectContext();
	*NewContext = *this;
	NewContext->AddInstigator(Instigator.Get(), EffectCauser.Get());
	if (GetHitResult())
	{
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

UScriptStruct* FCAP_GameplayEffectContext::GetScriptStruct() const
{
	return FCAP_GameplayEffectContext::StaticStruct();
}
