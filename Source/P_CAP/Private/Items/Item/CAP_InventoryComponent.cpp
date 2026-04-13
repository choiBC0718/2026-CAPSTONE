// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item/CAP_InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAP_ItemInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_SynergyTypes.h"
#include "Interface/CAP_InteractInterface.h"


UCAP_InventoryComponent::UCAP_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCAP_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ACAP_PlayerCharacter>(GetOwner());
	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);

	if (SynergyDataTable)
	{
		TArray<FSynergyDataTable*> AllRows;
		SynergyDataTable->GetAllRows<FSynergyDataTable>("",AllRows);

		for (FSynergyDataTable* Row : AllRows)
		{
			if (Row && Row->SynergyTag.IsValid())
			{
				SynergyDataCache.Add(Row->SynergyTag, Row);
			}
		}
	}
}

bool UCAP_InventoryComponent::AddItem(class UCAP_ItemInstance* NewItem)
{
	if (!NewItem)
		return false;

	if (InventoryItems.Num() >= Capacity)
	{
		if (OnInventoryFull.IsBound())
			OnInventoryFull.Broadcast(NewItem);
		return false;
	}

	// 아이템 추가 후 새로고침
	InventoryItems.Add(NewItem);
	RefreshSynergies();
	if (OnInventoryChanged.IsBound())
	{
		OnInventoryChanged.Broadcast(NewItem, true);
	}
	return true;
}

void UCAP_InventoryComponent::ProcessInteractInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	float HoldDuration = 1.f;
	if (!Player)
		return;
	
	ICAP_InteractInterface* InteractableObj = Cast<ICAP_InteractInterface>(NearbyInteractable);
	if (!InteractableObj)
		return;

	if (TriggerEvent == ETriggerEvent::Ongoing)
	{
		float Progress = FMath::Clamp(ElapsedTime / HoldDuration, 0.0f, 1.0f);
		Player->UpdateInteractProgress(Progress);
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		if (NearbyInteractable)
		{
			InteractableObj->InteractDisassemble(Player);
			Player->UpdateInteractProgress(0.f);
		}
	}
	else if (TriggerEvent == ETriggerEvent::Completed || TriggerEvent == ETriggerEvent::Canceled)
	{
		if (ElapsedTime < ItemEquipTriggerTime && NearbyInteractable)
		{
			InteractableObj->InteractEquip(Player);
		}
		Player->UpdateInteractProgress(0.f);
	}
}

void UCAP_InventoryComponent::RefreshSynergies()
{
	if (SynergyDataCache.IsEmpty())
	{
		UE_LOG(LogTemp,Warning, TEXT("인벤토리 컴포넌트에 시너지 데이터 테이블 넣어 - RefreshSynergies"));
		return;
	}

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
		if (!ItemInst)
			continue;
		const UCAP_ItemDataAsset* ItemData = ItemInst->GetItemDA();
		if (ItemData)
		{
			AddSynergyTag(ItemData->SynergyTag1);
			AddSynergyTag(ItemData->SynergyTag2);
		}
	}
	// 시너지 Effect 새로고침
	UpdateSynergyEffects();
}

bool UCAP_InventoryComponent::SwapItem(class UCAP_ItemInstance* OldItem, class UCAP_ItemInstance* NewItem)
{
	if (!OldItem || !NewItem)
		return false;

	int32 Index = InventoryItems.Find(OldItem);
	if (Index != INDEX_NONE)
	{
		InventoryItems[Index] = NewItem;
		RefreshSynergies();
		if (OnInventoryChanged.IsBound())
			OnInventoryChanged.Broadcast(NewItem, true);
		return true;
	}
	return false;
}

void UCAP_InventoryComponent::UpdateSynergyEffects()
{
	if (!OwnerASC || SynergyDataCache.IsEmpty())
	{
		UE_LOG(LogTemp,Warning, TEXT("인벤토리 컴포넌트에 시너지 데이터 테이블 넣어 - UpdateSynergyEffects"));
		return;
	}

	for (const TPair<FGameplayTag, FSynergyDataTable*>& Pair : SynergyDataCache )
	{
		FSynergyDataTable* Row = Pair.Value;
		FGameplayTag TargetTag = Pair.Key;
		
		const int32* FoundCurrent = CurrentSynergyCounts.Find(TargetTag);
		int32 CurrentCount = FoundCurrent ? *FoundCurrent : 0;

		const int32* FoundLast = CachedSynergyCounts.Find(TargetTag);
		int32 LastCount = FoundLast ? *FoundLast : -1;

		// 태그 개수 변화 없다면 패스
		if (CurrentCount == LastCount)	continue;

		// 개수 변했다면 이전의 시너지 버프 일단 모두 제거
		if (AppliedSynergyHandles.Contains(TargetTag))
		{
			for (FActiveGameplayEffectHandle& Handle : AppliedSynergyHandles[TargetTag])
			{
				OwnerASC->RemoveActiveGameplayEffect(Handle);
			}
			AppliedSynergyHandles[TargetTag].Empty();
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
						FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
						FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(EffectToApply, 1.f, Context);
						if (Spec.IsValid())
						{
							FActiveGameplayEffectHandle ActiveHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
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
