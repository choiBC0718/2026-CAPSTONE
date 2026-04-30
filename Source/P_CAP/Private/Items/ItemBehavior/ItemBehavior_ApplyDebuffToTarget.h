// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "Items/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "ItemBehavior_ApplyDebuffToTarget.generated.h"

/**
 * 
 */
UCLASS()
class UItemBehavior_ApplyDebuffToTarget : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	// 효과를 발동시킬 조건의 트리거 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Item.Trigger"), Category="Trigger")
	FGameplayTag TriggerEventTag;
	// 효과를 발동시킬 확률
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="100.0"), Category="Trigger")
	float TriggerChance = 100.f;
	// n번의 트리거를 발동시킨 뒤에 효과 발동
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1"), Category="Trigger")
	int32 RequiredTriggerCount = 1;
	// 최대 스택 수
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1"), Category="Effect")
	int32 MaxStackCount = 1;
	// 깎아낼 몬스터의 스탯 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Data.ItemStat"), Category="Effect")
	FGameplayTag TargetStatTag;
	// 최종 버프 계산 값 = BaseValue + (ScaleAttribute의 수치 * Magnitude)
	UPROPERTY(EditDefaultsOnly, Category="Value")
	float BaseValue=0.f;
	UPROPERTY(EditDefaultsOnly, Category="Value")
	FGameplayAttribute ScaleAttribute;
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
	void ApplyDebuffToSingleTarget(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC) const;
	int32 GetExistingStackCountOnTarget(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> MasterGE, FActiveGameplayEffectHandle& OutHandle) const;
};
