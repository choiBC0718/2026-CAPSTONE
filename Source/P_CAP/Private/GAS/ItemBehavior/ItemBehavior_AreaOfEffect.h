// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "ItemBehavior_AreaOfEffect.generated.h"

/**
 * 
 */
UCLASS()
class UItemBehavior_AreaOfEffect : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag TriggerEventTag;
	
	// 효과 부여 반경
	UPROPERTY(EditDefaultsOnly)
	float Radius = 500.f;

	UPROPERTY(EditDefaultsOnly)
	FGameplayAttribute DamageBasedAttribute;
	
	UPROPERTY(EditDefaultsOnly)
	float DamageMultiplier = 1.0f;

	// 동적으로 주입할 상태이상 태그들 (기절, 출혈 등)
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer StatusTagsToGrant;

	virtual void OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const override;
	virtual void OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const override;
	
	virtual void OnEventReceived(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const override;
};
