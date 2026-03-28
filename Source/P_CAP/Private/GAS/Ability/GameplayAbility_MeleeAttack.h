// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GameplayAbility_MeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_MeleeAttack : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_MeleeAttack();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;


protected:
	UFUNCTION()
	void OnDamageTagReceived(FGameplayEventData Payload);

	
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;
};
