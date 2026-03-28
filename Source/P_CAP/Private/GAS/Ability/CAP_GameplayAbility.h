// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CAP_GameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCAP_GameplayAbility();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	class UAnimMontage* AbilityMontage;
	
	void ApplyGameplayEffectToHitResult(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level=1);

	UAnimInstance* GetOwnerAnimInstance() const;
	
	UPROPERTY();
	FGameplayTag DamageTag;
};
