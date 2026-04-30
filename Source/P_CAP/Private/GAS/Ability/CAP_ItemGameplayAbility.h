// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/CAP_ItemDataAsset.h"
#include "CAP_ItemGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_ItemGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCAP_ItemGameplayAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	const class UCAP_ItemDataAsset* GetItemData();
};
