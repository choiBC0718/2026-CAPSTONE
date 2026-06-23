// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GA_PassiveItemMonitor.generated.h"

/**
 * 
 */
UCLASS()
class UGA_PassiveItemMonitor : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PassiveItemMonitor();

	// 아이템 행동으로부터 데이터를 전달받아 초기화하는 함수
	void InitMonitorData(const struct FItemConditionData& InCondition, const struct FItemActionData& InAction);

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	// 전달받은 데이터를 저장할 변수
	FItemConditionData ConditionData;
	FItemActionData ActionData;

	// 스탯 변경 이벤트를 수신할 콜백 함수
	void OnAttributeChanged(const struct FOnAttributeChangeData& ChangeData);

	// 현재 스탯 상태를 평가하여 GE를 적용/해제하는 로직
	void EvaluateCondition();

	// 씌워둔 마스터 버프의 핸들 (조건 불만족 시 벗기기 위함)
	FActiveGameplayEffectHandle ActiveBuffHandle;

	// 현재 버프가 활성화되어 있는지 여부
	bool bIsConditionMet = false;
};
