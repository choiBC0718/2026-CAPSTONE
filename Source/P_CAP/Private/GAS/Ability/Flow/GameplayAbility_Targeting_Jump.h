// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/Flow/GA_FlowBase.h"
#include "GameplayAbility_Targeting_Jump.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_Targeting_Jump : public UGA_FlowBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Targeting_Jump();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ACAP_TargetActor> TargetActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<class ACAP_TargetRangeIndicator> RangeIndicatorClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float MaxTargetingRange = 800.f;
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float DamageRange = 200.f;

	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float TickRotToCursorSpeed = 500.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	class UAnimMontage* CastMontage;

	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void OnJumpTagReceived(FGameplayEventData Payload);
	
private:
	FVector CachedTargetLocation;
	
	UPROPERTY()
	class UAbilityTask_TickRotToCursor* RotToCursor;
	UPROPERTY()
	class ACAP_TargetRangeIndicator* SpawnedRangeIndicator;
};
