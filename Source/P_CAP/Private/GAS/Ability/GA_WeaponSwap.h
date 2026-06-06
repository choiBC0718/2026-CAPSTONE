// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GA_WeaponSwap.generated.h"

/**
 * 
 */
UCLASS()
class UGA_WeaponSwap : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGA_WeaponSwap();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FGameplayTag SwapCooldownTag;
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float SwapCooldownTime = 1.0f;

private:
	void ApplySwapCooldown();
};
