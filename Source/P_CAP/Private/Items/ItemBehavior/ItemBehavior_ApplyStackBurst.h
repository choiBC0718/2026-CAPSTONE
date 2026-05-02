// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Items/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "ItemBehavior_ApplyStackBurst.generated.h"

/**
 * 스택을 쌓아 터뜨리는 효과
 */
UCLASS()
class UItemBehavior_ApplyStackBurst : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	// 효과를 발동시킬 조건의 트리거 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Item.Trigger"), Category="Trigger")
	FGameplayTag TriggerEventTag;
	// 효과를 발동시킬 확률
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="100.0"), Category="Trigger")
	float TriggerChance = 5.f;
	// 발동을 위해 필요한 트리거 발동 횟수
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1"), Category="Trigger")
	int32 BurstStackCount=1;
	
	// 스택 부여한 클래스 구별하기 위한 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="State.Debuff"), Category="Effect")
	FGameplayTag DynamicTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Value")
	ESkillDamageType DamageType = ESkillDamageType::Physical;
	// 최종 버프 계산 값 = BaseValue + (ScaleAttribute의 수치 * Magnitude)
	UPROPERTY(EditDefaultsOnly, Category="Value")
	float BaseValue=0.f;
	// 스탯의 비례 계수 (10% = 0.1)
	UPROPERTY(EditDefaultsOnly, Category="Value")
	float Magnitude=0.f;

	virtual void OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const override;
	virtual void OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const override;

protected:
	virtual void OnEventReceived(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const override;

private:
	bool CheckTriggerCondition(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const;
	void ApplyBurstLogicToSingleTarget(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC) const;
	
	int32 GetExistingMarkStackCount(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> MarkGE, FActiveGameplayEffectHandle& OutHandle) const;
};
