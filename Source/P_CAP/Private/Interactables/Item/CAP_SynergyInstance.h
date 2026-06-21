// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Interface/CAP_BehaviorStateProvider.h"
#include "Interface/CAP_BuffVisualInterface.h"
#include "UObject/NoExportTypes.h"
#include "CAP_SynergyInstance.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_SynergyInstance : public UObject, public ICAP_BehaviorStateProvider, public ICAP_BuffVisualInterface
{
	GENERATED_BODY()

public:
	void InitializeSynergy(FGameplayTag InSynergyTag, int32 InLv, const TArray<UCAP_ItemBehaviorBase*>& BehaviorClasses);

	virtual FBuffDisplayData GetBuffDisplayData(const FGameplayTag& EffectTag) const override;
	virtual FGameplayTag GetUniqueVisualID() const override {return SynergyTag;}
	
	//Behavior State Provider 인터페이스
	virtual float GetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior) const override;
	virtual void SetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior, float Time) override;

	virtual int32 GetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior) const override;
	virtual void AddBehaviorCount(const UCAP_ItemBehaviorBase* Behavior, int32 AddCount) override;
	virtual void ResetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior) override;

	virtual float GetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target) const override;
	virtual void SetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target, float Time) override;
	
	virtual TArray<FDelegateHandle>* GetBoundEventHandles(const UCAP_ItemBehaviorBase* Behavior) override;
	virtual const TArray<UCAP_ItemBehaviorBase*>& GetBehaviors() const override {return InstancedBehaviors;}
	virtual UObject* GetProviderObject() override {return this;}

	FGameplayTag SynergyTag;
	int32 SynergyLv=0;

private:
	UPROPERTY()
	TArray<UCAP_ItemBehaviorBase*> InstancedBehaviors;

	TMap<const UCAP_ItemBehaviorBase*, TArray<FDelegateHandle>> BoundEventHandles;
	TMap<TPair<const UCAP_ItemBehaviorBase*, TWeakObjectPtr<AActor>>, float> TargetCooldowns;
	UPROPERTY()	TMap<const UCAP_ItemBehaviorBase*, int32> BehaviorCounters;
	UPROPERTY()	TMap<const UCAP_ItemBehaviorBase*, float> BehaviorLastTriggerTimes;
};
