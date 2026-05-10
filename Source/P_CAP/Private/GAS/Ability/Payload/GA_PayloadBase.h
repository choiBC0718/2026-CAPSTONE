// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GA_PayloadBase.generated.h"

/**
 * 스킬 결과 처리 클래스
 * 데미지 적용, 투사체 생성, 캐릭터 루트모션 등 실행
 * Flow(입력 로직)를 통해 실행된 GA 또는 애니메이션에서 발생된 태그 이벤트를 트리거로 실행됨
 */
UCLASS(Abstract)
class UGA_PayloadBase : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PayloadBase(){}

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override
	{
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
		if (TriggerEventData)
		{
			ExecutePayloadLogic(*TriggerEventData);
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true,false);
	}
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual float GetPayloadTargetingRadius() {return 0.f;}
	
protected:
	/* 언리얼 C++ 자식 클래스에서 구현 강제를 위해 PURE_VIRTUAL(함수명, 반환값)
	 * 순수 가상 함수 (=0)를 객체를 만드려고 하여 발생하는 에러 방지
	 */
	virtual void ExecutePayloadLogic(const FGameplayEventData& EventData) PURE_VIRTUAL(UGA_PayloadBase::ExecutePayloadLogic, );
};
