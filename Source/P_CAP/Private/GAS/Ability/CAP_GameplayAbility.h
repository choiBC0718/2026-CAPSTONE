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
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	void ApplyGameplayEffectToHitResult(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level=1);
	void RotateToMouseCursor();
	
	UAnimInstance* GetOwnerAnimInstance() const;
	class ACAP_PlayerCharacter* GetPlayerCharacterFromActorInfo() const;
	class UCAP_WeaponDataAsset* GetWeaponDataAsset() const;
	const struct FWeaponSkillData* GetCurrentSkillData() const;
	
	FGameplayTag DamageTag;
	FGameplayTag RMSTag;
	UFUNCTION()
	void OnDamageTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnRMSTagReceived(FGameplayEventData Payload);
	
	UPROPERTY();
	class UAnimMontage* AbilityMontage;
	UPROPERTY();
	TSubclassOf<UGameplayEffect> AbilityDamageEffect;
};
