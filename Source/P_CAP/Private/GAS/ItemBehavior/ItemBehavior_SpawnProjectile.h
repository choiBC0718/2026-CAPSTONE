// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Actors/CAP_AoESpawner.h"
#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "ItemBehavior_SpawnProjectile.generated.h"

/**
 * 
 */
UCLASS()
class UItemBehavior_SpawnProjectile : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	// 효과를 발동시킬 조건의 트리거 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Item.Trigger"), Category="Trigger")
	FGameplayTag TriggerEventTag;
	// 발동위한 트리거 발동 횟수
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1"), Category="Trigger")
	int32 RequiredTriggerCount = 1;
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="100.0"), Category="Trigger")
	float TriggerChance = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Setting")
	TSubclassOf<class ACAP_AoESpawner> SpawnerClass;
	UPROPERTY(EditDefaultsOnly, Category="Setting")
	FAoESpawnerSetupData SpawnerData;

	virtual void OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const override;
	virtual void OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const override;

	virtual void OnEventReceived(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const override;
private:
	bool CheckTriggerCondition(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const;
};
