// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Item/CAP_ItemInstance.h"

void UCAP_ItemInstance::Initialize(UCAP_ItemDataBase* NewItemDA)
{
	ItemDA = NewItemDA;
	CurrentGrade = ItemDA ? ItemDA->ItemGrade : EItemGrade::Normal;
}


FBuffDisplayData UCAP_ItemInstance::GetBuffDisplayData(const FGameplayTag& EffectTag) const
{
	FBuffDisplayData BuffData;
	if (ItemDA)
		BuffData.Icon = ItemDA->ItemIcon;
	
	return BuffData;
}

float UCAP_ItemInstance::GetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior) const
{
	const float* TimePtr = BehaviorLastTriggerTimes.Find(Behavior);
	return TimePtr ? *TimePtr : -999.f;
}

void UCAP_ItemInstance::SetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior, float Time)
{
	BehaviorLastTriggerTimes.FindOrAdd(Behavior) = Time;
}

int32 UCAP_ItemInstance::GetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior) const
{
	const int32* CountPtr = BehaviorCounters.Find(Behavior);
	return CountPtr ? *CountPtr : 0;
}

void UCAP_ItemInstance::AddBehaviorCount(const UCAP_ItemBehaviorBase* Behavior, int32 AddCount)
{
	BehaviorCounters.FindOrAdd(Behavior) += AddCount;
}

void UCAP_ItemInstance::ResetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior)
{
	BehaviorCounters.FindOrAdd(Behavior) = 0;
}

float UCAP_ItemInstance::GetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target) const
{
	if (!Target)
		return -999.f;
	TWeakObjectPtr<AActor> WeakTarget(Target);
	TPair<const UCAP_ItemBehaviorBase*, TWeakObjectPtr<AActor>> Key(Behavior, WeakTarget);

	const float* TimerPtr = TargetCooldowns.Find(Key);
	return TimerPtr ? *TimerPtr : -999.f;
}

void UCAP_ItemInstance::SetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target, float Time)
{
	if (!Target)
		return;
	TWeakObjectPtr<AActor> WeakTarget(Target);
	TPair<const UCAP_ItemBehaviorBase*, TWeakObjectPtr<AActor>> Key(Behavior, WeakTarget);
	TargetCooldowns.FindOrAdd(Key) = Time;
	
	for (auto It = TargetCooldowns.CreateIterator(); It; ++It)
	{
		if (!It.Key().Value.IsValid() || It.Value()<= Target->GetWorld()->GetTimeSeconds())
			It.RemoveCurrent();
	}
}

TArray<FDelegateHandle>* UCAP_ItemInstance::GetBoundEventHandles(const UCAP_ItemBehaviorBase* Behavior)
{
	return &BoundEventHandles.FindOrAdd(Behavior);
}

const TArray<UCAP_ItemBehaviorBase*>& UCAP_ItemInstance::GetBehaviors() const
{
	static TArray<UCAP_ItemBehaviorBase*> EmptyArray;
	if (UCAP_ItemDataAsset* EquipDA = Cast<UCAP_ItemDataAsset>(ItemDA))
	{
		return EquipDA->ItemBehaviors;
	}
	return EmptyArray;
}

TArray<FGameplayAbilitySpecHandle>* UCAP_ItemInstance::GetGrantedAbilityHandles(const UCAP_ItemBehaviorBase* Behavior)
{
	if (!Behavior)
		return nullptr;
	return &GrantedAbilityHandlesMap.FindOrAdd(Behavior);
}

