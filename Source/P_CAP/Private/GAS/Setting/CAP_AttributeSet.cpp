// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting/CAP_AttributeSet.h"

#include "GameplayEffectExtension.h"

void UCAP_AttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
}

void UCAP_AttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0, GetMaxHealth()));
	}
}
