// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GA_Dodge.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Dodge : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Dodge();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	class UCAP_WeaponComponent* GetWeaponComponentFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const;

	UPROPERTY(EditDefaultsOnly, Category="Dodge")
	class UAnimMontage* DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category="Dodge")
	float DodgeSpeed = 1500.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Dodge")
	float DodgeDuration = 0.3f;

private:
	FGameplayTag DodgeCastTag;
};
