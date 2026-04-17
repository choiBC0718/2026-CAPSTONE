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

protected:
	UAnimInstance* GetOwnerAnimInstance() const;
	class ACAP_PlayerCharacter* GetPlayerCharacterFromActorInfo() const;
	const struct FWeaponSkillData* GetCurrentSkillData() const;

	class ACAP_ProjectileBase* SpawnProjectile(FVector SpawnLocation);
	FVector GetMuzzleSocketLocation(FName SocketName);
	void SendGameplayCueEvent(FHitResult HitResult);

	FGameplayTag DamageTag;
	FGameplayTag RMSTag;
	FGameplayTag SpawnProjectileTag;
	UFUNCTION()
	void OnDamageTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnRMSTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnSpawnProjectileTagReceived(FGameplayEventData Payload);
	
	UPROPERTY();
	class UAnimMontage* AbilityMontage;
	UPROPERTY();
	TSubclassOf<UGameplayEffect> AbilityDamageEffect;
	
	const FWeaponSkillData* SkillData;

	bool bIsCasting = false;
	FVector CachedTargetLocation;
	UFUNCTION()
	virtual void OnMontageInterrupted();
};
