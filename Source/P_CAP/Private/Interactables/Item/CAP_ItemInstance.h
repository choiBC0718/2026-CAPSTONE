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
class UCAP_ItemInstance : public UObject, public ICAP_BuffVisualInterface
{
	GENERATED_BODY()

public:
	void Initialize(UCAP_ItemDataBase* NewItemDA);

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

	void SetCachedASC(UCAP_AbilitySystemComponent* ASC);
	UCAP_AbilitySystemComponent* GetCachedASC() const;

	virtual FBuffDisplayData GetBuffDisplayData(const FGameplayTag& EffectTag) const override;

	EItemGrade GetCurrentGrade() const {return CurrentGrade;}
	void SetCurrentGrade(EItemGrade NewGrade) {CurrentGrade = NewGrade;}
	
protected:
	UPROPERTY()
	UCAP_ItemDataBase* ItemDA;

	EItemGrade CurrentGrade;

private:
	TWeakObjectPtr<UCAP_AbilitySystemComponent> CachedOwnerASC;
};
