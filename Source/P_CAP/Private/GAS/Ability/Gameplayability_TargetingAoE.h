// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "Gameplayability_TargetingAoE.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayability_TargetingAoE : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayability_TargetingAoE();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
private:
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& Data);

	UPROPERTY()
	TSubclassOf<class ACAP_TargetActor> TargetActorClass;
	UPROPERTY()
	class ACAP_TargetRangeIndicator* SpawnedRangeIndicator;

	void ProjectileTargetingConfirmed();
	void InstantTargetingConfirmed(const FGameplayAbilityTargetDataHandle& Data);
};
