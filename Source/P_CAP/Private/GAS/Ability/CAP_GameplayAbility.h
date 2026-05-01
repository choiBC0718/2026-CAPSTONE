// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CAP_GameplayAbility.generated.h"

/**
 * 가장 기본적인 역할 수행 클래스
 * 몽타주 재생 & 데미지 태그 이벤트 & 캐릭터 회전 & 루트모션
 */
UCLASS()
class UCAP_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCAP_GameplayAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
protected:
	UAnimInstance* GetOwnerAnimInstance() const;
	class ACAP_PlayerCharacter* GetPlayerCharacterFromActorInfo() const;
	const struct FWeaponSkillData* GetSkillDataFromContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	TArray<class ACAP_ProjectileBase*> SpawnProjectile(FVector SpawnLocation, const struct FWeaponSkillData* InSkillData);
	FVector GetMuzzleSocketLocation(FName SocketName);
	void SendGameplayCueEvent(FHitResult HitResult, const struct FWeaponSkillData* InSkillData);

	FGameplayTag DamageMultiplierDataTag;
	FGameplayTag ChargeMultiplierDataTag;
	
	FGameplayTag DamageTag;
	FGameplayTag RMSTag;
	FGameplayTag SpawnProjectileTag;
	FGameplayTag DataCooldownTag;
	UFUNCTION()
	void OnDamageTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnRMSTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnSpawnProjectileTagReceived(FGameplayEventData Payload);
	
	bool bIsCasting = false;
	FVector CachedTargetLocation;
	UFUNCTION()
	virtual void OnMontageInterrupted();

	TSubclassOf<UGameplayEffect> GetDamageGE() const;

	void SendItemTriggerEvent(bool bIsHit, FGameplayAbilityTargetDataHandle TargetData = FGameplayAbilityTargetDataHandle());
	bool IsBasicAttack() const;

	float ChargedTime = 1.f;
};
