// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Item/CAP_SynergyInstance.h"

#include "Framework/Subsystem/CAP_SynergySubsystem.h"
#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"

void UCAP_SynergyInstance::InitializeSynergy(FGameplayTag InSynergyTag, int32 InLv,
                                             const TArray<UCAP_ItemBehaviorBase*>& BehaviorClasses)
{
	SynergyTag = InSynergyTag;
	SynergyLv = InLv;

	for (UCAP_ItemBehaviorBase* TemplateBehavior : BehaviorClasses)
	{
		if (TemplateBehavior)
			if (UCAP_ItemBehaviorBase* NewBehavior = DuplicateObject<UCAP_ItemBehaviorBase>(TemplateBehavior, this))
				InstancedBehaviors.Add(NewBehavior);
	}
}

FBuffDisplayData UCAP_SynergyInstance::GetBuffDisplayData(const FGameplayTag& EffectTag) const
{
	FBuffDisplayData Data;
	if (UWorld* World = GetWorld())
	{
		if (UCAP_SynergySubsystem* SynSubsys = World->GetGameInstance()->GetSubsystem<UCAP_SynergySubsystem>())
		{
			if (SynSubsys->SynergyMap.Contains(SynergyTag))
			{
				if (UCAP_SynergyDataAsset* SynDA = SynSubsys->SynergyMap[SynergyTag].LoadSynchronous())
					Data.Icon = SynDA->SynergyIcon;
			}
		}
	}
	return Data;
}

float UCAP_SynergyInstance::GetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior) const
{
	const float* TimePtr = BehaviorLastTriggerTimes.Find(Behavior);
	return TimePtr ? *TimePtr : -999.f;
}

void UCAP_SynergyInstance::SetBehaviorLastTriggerTime(const UCAP_ItemBehaviorBase* Behavior, float Time)
{
	BehaviorLastTriggerTimes.FindOrAdd(Behavior) = Time;
}

int32 UCAP_SynergyInstance::GetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior) const
{
	const int32* CountPtr = BehaviorCounters.Find(Behavior);
	return CountPtr ? *CountPtr : 0;
}

void UCAP_SynergyInstance::AddBehaviorCount(const UCAP_ItemBehaviorBase* Behavior, int32 AddCount)
{
	BehaviorCounters.FindOrAdd(Behavior) += AddCount;
}

void UCAP_SynergyInstance::ResetBehaviorCount(const UCAP_ItemBehaviorBase* Behavior)
{
	BehaviorCounters.FindOrAdd(Behavior) = 0;
}

float UCAP_SynergyInstance::GetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target) const
{
	if (!Target) return -999.f;
	TWeakObjectPtr<AActor> WeakTarget(Target);
	TPair<const UCAP_ItemBehaviorBase*, TWeakObjectPtr<AActor>> Key(Behavior, WeakTarget);
	const float* TimePtr = TargetCooldowns.Find(Key);
	return TimePtr ? *TimePtr : -999.f;
}

void UCAP_SynergyInstance::SetBehaviorTargetCooldown(const UCAP_ItemBehaviorBase* Behavior, AActor* Target, float Time)
{
	if (!Target) return;
	TWeakObjectPtr<AActor> WeakTarget(Target);
	TPair<const UCAP_ItemBehaviorBase*, TWeakObjectPtr<AActor>> Key(Behavior, WeakTarget);
	TargetCooldowns.FindOrAdd(Key) = Time;
}

TArray<FDelegateHandle>* UCAP_SynergyInstance::GetBoundEventHandles(const UCAP_ItemBehaviorBase* Behavior)
{
	return &BoundEventHandles.FindOrAdd(Behavior);
}

TArray<FGameplayAbilitySpecHandle>* UCAP_SynergyInstance::GetGrantedAbilityHandles(const UCAP_ItemBehaviorBase* Behavior)
{
	if (!Behavior)
		return nullptr;
		
	return &GrantedAbilityHandlesMap.FindOrAdd(Behavior);
}
