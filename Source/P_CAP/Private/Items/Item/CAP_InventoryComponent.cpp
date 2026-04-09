// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item/CAP_InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAP_ItemInstance.h"
#include "Data/CAP_SynergyTypes.h"


UCAP_InventoryComponent::UCAP_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UCAP_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}

bool UCAP_InventoryComponent::AddItem(class UCAP_ItemInstance* NewItem)
{
	if (!NewItem)
		return false;

	if (InventoryItems.Num() >= Capacity)
		return false;

	// 아이템 추가 후 새로고침
	InventoryItems.Add(NewItem);
	RefreshSynergies();
	if (OnInventoryChanged.IsBound())
	{
		OnInventoryChanged.Broadcast(NewItem, true);
	}
	return true;
}

void UCAP_InventoryComponent::RefreshSynergies()
{
	if (!SynergyDataTable)
		return;

	// 일단 비워
	CurrentSynergyCounts.Empty();

	auto AddSynergyTag = [&](const FGameplayTag& Tag)
	{	// 매개변수 태그 = Key로 하여 Value 증가
		if (Tag.IsValid())
		{
			int32& Count = CurrentSynergyCounts.FindOrAdd(Tag);
			Count++;
		}
	};

	for (UCAP_ItemInstance* ItemInst : InventoryItems)
	{
		if (ItemInst && ItemInst->GetItemDA())
		{
			if (ItemInst->GetItemDA()->SynergyTag1.IsValid())
				AddSynergyTag(ItemInst->GetItemDA()->SynergyTag1);
			if (ItemInst->GetItemDA()->SynergyTag2.IsValid())
				AddSynergyTag(ItemInst->GetItemDA()->SynergyTag2);
		}
	}
	// 시너지 Effect 새로고침
	UpdateSynergyEffects();
}

void UCAP_InventoryComponent::UpdateSynergyEffects()
{
	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC || !SynergyDataTable)
		return;

	// SynergyDataTable의 모든 행 저장
	TArray<FSynergyDataTable*> AllSynergies;
	SynergyDataTable->GetAllRows<FSynergyDataTable>("", AllSynergies);

	for (FSynergyDataTable* Row : AllSynergies)
	{
		// 태그 설정 안했으면 패스
		if (!Row || !Row->SynergyTag.IsValid())
			continue;

		FGameplayTag TargetTag = Row->SynergyTag;
		int32 CurrentCount = CurrentSynergyCounts.Contains(TargetTag) ? CurrentSynergyCounts[TargetTag] : 0;
		int32 LastCount = CachedSynergyCounts.Contains(TargetTag) ? CachedSynergyCounts[TargetTag] : -1;

		// 태그 개수 변화 없다면 패스
		if (CurrentCount == LastCount)	continue;

		// 개수 변했다면 이전의 시너지 버프 일단 모두 제거
		if (AppliedSynergyHandles.Contains(TargetTag))
		{
			for (FActiveGameplayEffectHandle& Handle : AppliedSynergyHandles[TargetTag])
			{
				ASC->RemoveActiveGameplayEffect(Handle);
			}
			AppliedSynergyHandles.Empty();
		}
		else
		{
			AppliedSynergyHandles.Add(TargetTag, TArray<FActiveGameplayEffectHandle>());
		}

		// 현재 개수에 맞게 모든 버프 다시 제공
		if (CurrentCount > 0)
		{
			for (int i=0 ; i<Row->SynergyLevels.Num() ; i++)
			{
				if (CurrentCount >= Row->SynergyLevels[i].RequiredCount)
				{
					TSubclassOf<UGameplayEffect> EffectToApply = Row->SynergyLevels[i].SynergyEffect;
					if (EffectToApply)
					{
						FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
						FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EffectToApply, 1.f, Context);
						if (Spec.IsValid())
						{
							FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
							AppliedSynergyHandles[TargetTag].Add(ActiveHandle);
						}
					}
				}
			}
			// 현재 개수 저장
			CachedSynergyCounts.Add(TargetTag, CurrentCount);
		}
		// 아이템 모두 제거 시 완전 삭제
		else
		{
			CachedSynergyCounts.Remove(TargetTag);
			AppliedSynergyHandles.Remove(TargetTag);
		}
	}
}
