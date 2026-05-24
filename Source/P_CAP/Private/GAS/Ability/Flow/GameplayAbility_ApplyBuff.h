// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/Flow/GA_FlowBase.h"
#include "GameplayAbility_ApplyBuff.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_ApplyBuff : public UGA_FlowBase
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void OnBuffTagReceived(FGameplayEventData Payload);
};
