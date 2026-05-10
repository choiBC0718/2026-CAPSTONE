// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_FlowBase.h"
#include "Gameplayability_TargetingAoE.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayability_TargetingAoE : public UGA_FlowBase
{
	GENERATED_BODY()

public:
	UGameplayability_TargetingAoE();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ACAP_TargetActor> TargetActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ACAP_TargetRangeIndicator> RangeIndicatorClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float MaxTargetingRange = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float TickRotToCursorSpeed = 500.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	class UAnimMontage* CastMontage;
	
private:
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& Data);
	
	//void ProjectileTargetingConfirmed();
	//void InstantTargetingConfirmed(const FGameplayAbilityTargetDataHandle& Data);

	FGameplayTag ConfirmTag;
	
	UPROPERTY()
	class ACAP_TargetRangeIndicator* SpawnedRangeIndicator;
};
