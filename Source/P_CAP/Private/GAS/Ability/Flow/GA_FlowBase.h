// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GA_FlowBase.generated.h"

/**
 * 스킬 입력 로직 부모 클래스
 * 특별한 로직 필요없으면 일회성으로 해당 클래스 이용
 * 일회성, Combo, Hold, Charge, Targeting 등
 */
UCLASS()
class UGA_FlowBase : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGA_FlowBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	// 스킬 사용시에 마우스 방향으로 회전 속도
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	float RotateToCursorSpeed = 1500.f;

	float ChargedTime = 1.f;
	
	UFUNCTION()
	void OnRMSTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnAnimHitTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnAnimSpawnTagReceived(FGameplayEventData Payload);
	
	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* MontageTask;

	void ChangeCurrentMontagePlayRate(float PlayRate);
	
	FGameplayTag RMSTag;
	FGameplayTag DataCooldownTag;
	
	FGameplayTag AnimHitTag;
	FGameplayTag AnimSpawnTag;
	
	FGameplayTag DoDamageTag;
	FGameplayTag SpawnProjectileTag;

	FGameplayTag TriggerCastBasicTag;
	FGameplayTag TriggerCastAbilityTag;

private:
	bool bIsCollisionIgnored = false;
};
