// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CAP_BehaviorStateProvider.generated.h"


class UCAP_ItemBehaviorBase;

UINTERFACE(MinimalAPI)
class UCAP_BehaviorStateProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 아이템 인스턴스, 시너지 인스턴스에서 Behavior 쿨타임, 스택 관리용
 */
class ICAP_BehaviorStateProvider
{
	GENERATED_BODY()

public:
	// 쿨타임 관리
	virtual float GetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior) const = 0;
	virtual void SetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior, float Time) = 0;

	// 스택 관리
	virtual int32 GetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior) const = 0;
	virtual void AddBehaviorCount(const UCAP_ItemBehaviorBase* Behavior, int32 AddCount) = 0;
	virtual void ResetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior) = 0;

	virtual float GetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target) const = 0;
	virtual void SetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target, float Time) = 0;
	
	// 이벤트 바인딩 핸들 추적
	virtual TArray<FDelegateHandle>* GetBoundEventHandles(const UCAP_ItemBehaviorBase* Behavior) = 0;

	// 이 Provider가 가지고 있는 Behavior 목록 반환 (GA에서 루프 돌리기 위함)
	virtual const TArray<UCAP_ItemBehaviorBase*>& GetBehaviors() const = 0;

	// 자기 자신을 UObject 포인터로 반환 (델리게이트를 통해 UI로 쏠 때 식별용)
	virtual UObject* GetProviderObject() = 0;
};
