// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CAP_ItemGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_ItemGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCAP_ItemGameplayAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	void HandleTriggerLogic(const struct FItemSkillData& SkillData, int32 SkillIndex);
	void HandleAutoLogic(const struct FItemSkillData& SkillData, int32 SkillIndex);

	// 트리거 발동
	UFUNCTION()
	void OnTriggerEventReceived(FGameplayEventData Payload);
	// 쿨타임 돌때마다 발동
	UFUNCTION()
	void ExecuteAutoCast(int32 SkillIndex);

	// 실제 이펙트 소환 & 처리
	UFUNCTION()
	void ExecuteEffect(const struct FItemSkillData& SkillData, FGameplayEventData Payload);

	// 현재 누적된 트리거 횟수
	TMap<int32, int32> TriggerCounts;
	// 자동 시전 타이머 핸들
	TMap<int32, FTimerHandle> AutoCastTimer;

private:
	const class UCAP_ItemDataAsset* GetItemData();
};
