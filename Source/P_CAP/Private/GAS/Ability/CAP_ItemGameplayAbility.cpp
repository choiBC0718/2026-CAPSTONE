// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_ItemGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_ItemDataAsset.h"
#include "Items/Item/CAP_ItemInstance.h"

UCAP_ItemGameplayAbility::UCAP_ItemGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UCAP_ItemGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	// 아이템 착용 시점에 1회 호출
	const UCAP_ItemDataAsset* ItemDA = GetItemData();
	UCAP_ItemInstance* ItemInst = Cast<UCAP_ItemInstance>(GetCurrentAbilitySpec()->SourceObject);
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ItemDA || !ItemInst || !ASC)
	{
		K2_EndAbility();
		return;
	}

	for (UCAP_ItemBehaviorBase* Behavior : ItemDA->ItemBehaviors)
	{
		if (Behavior)
		{
			UE_LOG(LogTemp,Warning,TEXT("아이템 장착됨"));
			Behavior->OnEquipped(ItemInst,ASC);
		}
	}
}

void UCAP_ItemGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	const UCAP_ItemDataAsset* ItemDA = GetItemData();
	UCAP_ItemInstance* ItemInst = Cast<UCAP_ItemInstance>(GetCurrentAbilitySpec()->SourceObject);
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ItemDA && ItemInst && ASC)
	{
		for (UCAP_ItemBehaviorBase* Behavior : ItemDA->ItemBehaviors)
		{
			if (Behavior)
			{
				UE_LOG(LogTemp,Warning,TEXT("아이템 해제됨"));
				Behavior->OnUnequipped(ItemInst,ASC);
			}
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const class UCAP_ItemDataAsset* UCAP_ItemGameplayAbility::GetItemData()
{
	if (GetCurrentAbilitySpec() && GetCurrentAbilitySpec()->SourceObject.IsValid())
	{
		if (UCAP_ItemDataAsset* ItemDA = Cast<UCAP_ItemDataAsset>(GetCurrentAbilitySpec()->SourceObject.Get()))
			return ItemDA;
		if (UCAP_ItemInstance* ItemInst = Cast<UCAP_ItemInstance>(GetCurrentAbilitySpec()->SourceObject.Get()))
			return Cast<UCAP_ItemDataAsset>(ItemInst->GetItemDA());
	}
	return nullptr;
}
