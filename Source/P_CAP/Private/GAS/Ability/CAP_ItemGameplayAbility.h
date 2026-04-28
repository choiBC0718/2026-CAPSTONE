// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/CAP_ItemDataAsset.h"
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

	FGameplayTag ItemEffectDurationTag;
	FGameplayTag DataCooldownTag;

	// 효과 적용할 대상의 ASC 배열
	TArray<UAbilitySystemComponent*> GetTargetASCs(const struct FItemEffectPayload& Effect, const FGameplayEventData& Payload, UAbilitySystemComponent* SourceASC) const;
	// 기존 버프 탐색 및 데이터 추출
	void FindExistingStack(UAbilitySystemComponent* TargetASC, const struct FItemEffectPayload& Effect, class UCAP_ItemInstance* ItemInst, FActiveGameplayEffectHandle& OutOldHandle, int32& OutCurrentStacks, float& OutAppliedBonus) const;
	// 중첩 루프 방지가 적용된 깨끗한 최종 수치 계산
	float CalculateCleanMagnitude(UAbilitySystemComponent* SourceASC, const struct FItemEffectPayload& Effect, float AppliedBonus) const;
	// 새로운 GE 세팅 및 타겟에게 적용
	void ApplyEffectToTarget(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, const struct FItemEffectPayload& Effect, class UCAP_ItemInstance* ItemInst, float FinalMagnitude, int32 TargetStackCount, const FHitResult* HitResult, TSubclassOf<UGameplayEffect> MasterGE) const;
};
