// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "ItemBehavior_ApplyBuffToSelf.generated.h"

/**
 * 트리거 태그를 받아 플레이어에게 버프를 부여하는 클래스
 */
UCLASS()
class UItemBehavior_ApplyBuffToSelf : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	// 효과를 발동시킬 조건의 트리거 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Item.Trigger"), Category="Trigger")
	FGameplayTag TriggerEventTag;
	// n번의 트리거를 발동시킨 뒤에 효과 발동
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1"), Category="Trigger")
	int32 RequiredTriggerCount = 1;
	// RequiredTriggerCount를 증가시킬 확률
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="100.0"), Category="Trigger")
	float TriggerChance = 100.f;
	// 최대 스택 수
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1"), Category="Effect")
	int32 MaxStackCount = 1;
	// 버프로 스탯 올릴 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Data.ItemStat"), Category="Effect")
	FGameplayTag TargetStatTag;
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
	void ApplyBuffWithStack(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, int32 TargetStackCount) const;
	int32 GetExistingStackCount(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, FActiveGameplayEffectHandle& OutHandle) const;
};