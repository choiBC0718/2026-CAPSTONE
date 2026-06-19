// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GAS/Actors/CAP_ProjectileBase.h"
#include "GA_MonsterRangedAttack.generated.h"

USTRUCT(BlueprintType)
struct FMonsterRangedAttackEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack", meta=(ClampMin="0.0"))
	float Weight = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack", meta=(ClampMin="0.0"))
	float BaseDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack", meta=(ClampMin="0.0"))
	float DamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack")
	ESkillDamageType DamageType = ESkillDamageType::Physical;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack", meta=(ClampMin="0.1"))
	float PlayRate = 1.f;
};

UCLASS()
class UGA_MonsterRangedAttack : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MonsterRangedAttack();

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Ranged Attack")
	TArray<FMonsterRangedAttackEntry> AttackEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Ranged Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Ranged Attack")
	TSubclassOf<ACAP_ProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Ranged Attack")
	FName FireSocketName = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Ranged Attack")
	FVector SpawnOffset = FVector(80.f, 0.f, 60.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Ranged Attack", meta=(ClampMin="0.0"))
	float BaseDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Ranged Attack", meta=(ClampMin="0.0"))
	float DamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Ranged Attack")
	ESkillDamageType DamageType = ESkillDamageType::Physical;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Ranged Attack", meta=(ClampMin="0.1"))
	float MontagePlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Projectile")
	EProjectileType ProjectileType = EProjectileType::Straight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Projectile", meta=(ClampMin="0.0"))
	float ProjectileSpeed = 900.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Projectile", meta=(ClampMin="0.0"))
	float MaxDistance = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Projectile", meta=(ClampMin="0.0"))
	float ExplosionRadius = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Projectile", meta=(ClampMin="1"))
	int32 MaxHitCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Monster|Projectile", meta=(ClampMin="0.0"))
	float ArcTension = 0.5f;

	UFUNCTION()
	void OnSpawnProjectileEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackMontageFinished();

private:
	FMonsterRangedAttackEntry ResolveAttackEntry() const;
	bool SpawnProjectileAtTarget(const FGameplayEventData& Payload);
	FVector ResolveSpawnLocation(const FGameplayEventData& Payload) const;
	FRotator ResolveFireRotation(const FVector& SpawnLocation) const;
	FGameplayEffectSpecHandle MakeDamageSpec() const;
	void EndAttackByFallbackTimer();

	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentAttackMontage;

	FMonsterRangedAttackEntry CurrentAttackEntry;

	FTimerHandle AttackFallbackTimerHandle;
};
