// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_OverheadStatsGauge.h"

#include "AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Widget/Common/CAP_ValueGauge.h"

void UCAP_OverheadStatsGauge::ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
		return;

	if (HealthBar)
	{
		HealthBar->SetAndBoundToGameplayAttribute(
			AbilitySystemComponent,
			UCAP_AttributeSet::GetHealthAttribute(),
			UCAP_AttributeSet::GetMaxHealthAttribute());
	}
}
