// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_AbilityListView.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_AbilityListView : public UListView
{
	GENERATED_BODY()

public:
	void ConfigureAbilities(const TMap<EAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities);
	void RefreshWeaponSkills(class UCAP_WeaponInstance* WeaponInstance);
};
