// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_AbilityListView.h"

#include "Abilities/GameplayAbility.h"

void UCAP_AbilityListView::ConfigureAbilities(const TMap<EAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
	for (const TPair<EAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		AddItem(AbilityPair.Value.GetDefaultObject());
	}
}
