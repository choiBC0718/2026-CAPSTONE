// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "ItemBehavior_ApplyStatusToTarget.generated.h"

/**
 * 
 */
UCLASS()
class UItemBehavior_ApplyStatusToTarget : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Item.Trigger"))
	FGameplayTag TriggerEventTag;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="100.0"))
	float TriggerChance = 100.f;

	UPROPERTY(EditDefaultsOnly, meta=(Categories="State.Debuff"))
	FGameplayTagContainer StatusTagsToGrant;

	UPROPERTY(EditDefaultsOnly, Category="Value")
	float Duration = 0.f;
	
	virtual void OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const override;
	virtual void OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const override;

protected:
	virtual void OnEventReceived(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const override;

private:
	bool CheckTriggerCondition(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const;
	void ApplyStatusToSingleTarget(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC) const;
};
