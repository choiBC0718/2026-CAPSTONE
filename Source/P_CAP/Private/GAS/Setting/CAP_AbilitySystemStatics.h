// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CAP_AbilitySystemStatics.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_AbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FGameplayTag GetHealthFullStatTag();
	static FGameplayTag GetHealthEmptyStatTag();
	static FGameplayTag GetDeadStateTag();
	static FGameplayTag GetMovementBlockStateTag();

	static FGameplayTag GetBasicAttackTag();
	static FGameplayTag GetSkillAttackTag();
	static FGameplayTag GetDamageTag();
	static FGameplayTag GetRMSTag();
	static FGameplayTag GetSpawnProjectileTag();
	static FGameplayTag GetAbilityChargeTimeTag();

	static FGameplayTag GetDataDamageMultiplierTag();
	static FGameplayTag GetDataDamageBaseTag();
	static FGameplayTag GetDataDamageTag();
	static FGameplayTag GetDataCooldownTag();
	static FGameplayTag GetDataEffectDurationTag();
	static FGameplayTag GetDataStackTag();

	static FGameplayTag GetItemTriggerCastBasic();
	static FGameplayTag GetItemTriggerCastAbility();
	static FGameplayTag GetItemTriggerHitBasic();
	static FGameplayTag GetItemTriggerHitAbility();
};
