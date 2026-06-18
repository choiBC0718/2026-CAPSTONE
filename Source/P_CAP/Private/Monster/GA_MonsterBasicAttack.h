// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GA_MonsterBasicAttack.generated.h"

UCLASS()
class UGA_MonsterBasicAttack : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MonsterBasicAttack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Attack", meta=(ClampMin="0.0"))
	float BaseDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Attack", meta=(ClampMin="0.0"))
	float DamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Attack")
	ESkillDamageType DamageType = ESkillDamageType::Physical;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Attack", meta=(ClampMin="0.1"))
	float MontagePlayRate = 1.f;

	UFUNCTION()
	void OnAnimHitEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackMontageFinished();

private:
	bool ApplyDamageFromHitResult(const FHitResult& HitResult);
	void EndAttackByFallbackTimer();

	UPROPERTY()
	TSet<TObjectPtr<AActor>> DamagedActors;

	FTimerHandle AttackFallbackTimerHandle;
};
