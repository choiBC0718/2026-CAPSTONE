// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Items/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "ItemBehavior_ApplyDot.generated.h"

/**
 * 도트 데미지 부여 클래스
 */
UCLASS()
class UItemBehavior_ApplyDot : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	// 효과를 발동시킬 조건의 트리거 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Item.Trigger"), Category="Trigger")
	FGameplayTag TriggerEventTag;
	// 효과를 발동시킬 확률
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="100.0"), Category="Trigger")
	float TriggerChance = 5.f;

	// 최대 스택 수
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1"), Category="Effect")
	int32 MaxStackCount = 1;
	// 다른 DOT를 구별하기 위한 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="State.Debuff"), Category="Effect")
	FGameplayTag DynamicTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Value")
	ESkillDamageType DamageType = ESkillDamageType::Physical;
	
	UPROPERTY(EditDefaultsOnly, Category="Value")
	float BaseTickDamage=0.f;
	// 스탯의 비례 계수 (10% = 0.1)
	UPROPERTY(EditDefaultsOnly, Category="Value")
	float Magnitude=0.f;
	// 버프 유지 시간
	UPROPERTY(EditDefaultsOnly, Category="Value")
	float Duration = 0.f;

	virtual void OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const override;
	virtual void OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const override;

protected:
	virtual void OnEventReceived(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const override;

private:
	bool CheckTriggerCondition(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const;
	void ApplyDoTToSingleTarget(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC) const;
	
	// DynamicTag를 기반으로 기존 핸들을 찾는 특수 검색 함수
	int32 GetExistingDoTStackCount(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> MasterGE, FActiveGameplayEffectHandle& OutHandle) const;
};
