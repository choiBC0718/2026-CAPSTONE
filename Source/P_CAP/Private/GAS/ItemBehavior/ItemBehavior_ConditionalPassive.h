// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "ItemBehavior_ConditionalPassive.generated.h"

/**
 * 
 */
UCLASS()
class UItemBehavior_ConditionalPassive : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	// 기획자가 설정할 조건 데이터
	UPROPERTY(EditDefaultsOnly, Category = "Item|Condition")
	FItemConditionData ConditionData;

	// 기획자가 설정할 결과 데이터
	UPROPERTY(EditDefaultsOnly, Category = "Item|Action")
	FItemActionData ActionData;

	// 이 아이템이 장착될 때 캐릭터에게 부여할 감시용 패시브 스킬
	UPROPERTY(EditDefaultsOnly, Category = "Item|Ability")
	TSubclassOf<class UGA_PassiveItemMonitor> MonitorAbilityClass;

	virtual void OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const override;
	virtual void OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const override;
};
