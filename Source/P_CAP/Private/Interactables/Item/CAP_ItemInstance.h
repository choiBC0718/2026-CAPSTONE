// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_ItemDataAsset.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "Interface/CAP_BuffVisualInterface.h"
#include "CAP_ItemInstance.generated.h"

class UCAP_AbilitySystemComponent;
/**
 * 아이템 1개 자체에 대한 인스턴스
 */
UCLASS()
class UCAP_ItemInstance : public UObject, public ICAP_BuffVisualInterface, public ICAP_BehaviorStateProvider
{
	GENERATED_BODY()

public:
	virtual void Initialize(UCAP_ItemDataBase* NewItemDA);

	UFUNCTION()
	UCAP_ItemDataBase* GetItemDA() const {return ItemDA;}
	
	// 특정 아이템 효과가 구독한 이벤트 핸들 저장
	TMap<const class UCAP_ItemBehaviorBase*, TArray<FDelegateHandle>> BoundEventHandles;
	// 각 아이템 행동 모듈에서 효과를 발동 시키기 위해 트리거가 몇번 발동 되었는지 저장되는 Map
	UPROPERTY()
	TMap<const class UCAP_ItemBehaviorBase*, int32> BehaviorCounters;
	// 각 아이템 행동 모듈 별 마지막 발동 시간 저장 Map, 태그 이용시 같은 아이템에 대해 문제 발생하여 float로 사용
	UPROPERTY()
	TMap<const class UCAP_ItemBehaviorBase*,float> BehaviorLastTriggerTimes;
	TMap<TPair<const UCAP_ItemBehaviorBase*, TWeakObjectPtr<AActor>>, float> TargetCooldowns;

	EItemGrade GetCurrentGrade() const {return CurrentGrade;}
	
	//Buff Visual 인터페이스
	virtual FBuffDisplayData GetBuffDisplayData(const FGameplayTag& EffectTag) const override;
	virtual FGameplayTag GetUniqueVisualID() const override {return FGameplayTag();}

	//Behavior State Provider 인터페이스
	virtual float GetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior) const override;
	virtual void SetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior, float Time) override;

	virtual int32 GetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior) const override;
	virtual void AddBehaviorCount(const UCAP_ItemBehaviorBase* Behavior, int32 AddCount) override;
	virtual void ResetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior) override;

	virtual float GetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target) const override;
	virtual void SetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target, float Time) override;
	
	virtual TArray<FDelegateHandle>* GetBoundEventHandles(const UCAP_ItemBehaviorBase* Behavior) override;
	virtual const TArray<UCAP_ItemBehaviorBase*>& GetBehaviors() const override;
	virtual UObject* GetProviderObject() override {return this;}
	
protected:
	UPROPERTY()
	UCAP_ItemDataBase* ItemDA;

	EItemGrade CurrentGrade;
};
