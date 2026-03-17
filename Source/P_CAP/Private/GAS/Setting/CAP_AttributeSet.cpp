// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting/CAP_AttributeSet.h"

void UCAP_AttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
}
