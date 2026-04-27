// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item/CAP_InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAP_ItemInstance.h"
#include "GameplayTagsManager.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Data/CAP_SynergyTypes.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Ability/CAP_ItemGameplayAbility.h"
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
		SynergyDataCache.Reserve(AllRows.Num());

		for (FSynergyDataTable* Row : AllRows)
		{
			if (Row && Row->SynergyTag.IsValid())
			{
				SynergyDataCache.Add(Row->SynergyTag, Row);
			}
		}
	}
	FGameplayTag ParentTag = FGameplayTag::RequestGameplayTag(FName("Data.ItemStat"));
	CachedItemDataTags = UGameplayTagsManager::Get().RequestGameplayTagChildren(ParentTag);
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

	UE_LOG(LogTemp, Warning, TEXT("새로운 아이템 착용"));
	// 아이템 추가 후 새로고침
	InventoryItems.Add(NewItem);
	ApplyItemStatEffects(NewItem);
	RefreshSynergies();
	GiveItemAbility(NewItem);
	
	if (OnInventoryChanged.IsBound())
		OnInventoryChanged.Broadcast(NewItem, true);
	
	return true;
}

bool UCAP_InventoryComponent::SwapItem(class UCAP_ItemInstance* OldItem, class UCAP_ItemInstance* NewItem)
{
	if (!OldItem || !NewItem)
		return false;

	int32 Index = InventoryItems.Find(OldItem);
	if (Index != INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 스왑 로직 - 적용되어 있던 아이템 제거(효과 제거)"));
		RemoveItemEffect(OldItem);
		
		UE_LOG(LogTemp, Warning, TEXT("새로운 아이템 효과 적용"));
		InventoryItems[Index] = NewItem;
		RefreshSynergies();
		GiveItemAbility(NewItem);
		
		if (OnInventoryChanged.IsBound())
			OnInventoryChanged.Broadcast(NewItem, true);
		return true;
	}
	return false;
}

void UCAP_InventoryComponent::RemoveItemEffect(UCAP_ItemInstance* ItemInst)
{
	UE_LOG(LogTemp,Warning, TEXT("인벤토리에서 아이템 제거"));
	RemoveItemStatEffects(ItemInst);
	RemoveItemAbility(ItemInst);
}

void UCAP_InventoryComponent::RemoveItem(class UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst) return;

	RemoveItemEffect(ItemInst);
	InventoryItems.Remove(ItemInst);
	
	RefreshSynergies();
	
	if (OnInventoryChanged.IsBound())
		OnInventoryChanged.Broadcast(ItemInst, false);
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
	CurrentSynergyCounts.Empty(InventoryItems.Num());
	
	for (UCAP_ItemInstance* ItemInst : InventoryItems)
	{
		if (!ItemInst)
			continue;
		if (UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA())
		{
			const TArray<FGameplayTag> Synergies = ItemDA->GetSynergyTags();
			for (const FGameplayTag& Tag : Synergies)
			{
				CurrentSynergyCounts.FindOrAdd(Tag)++;
			}
		}
	}
	// 시너지 Effect 새로고침
	UpdateSynergyEffects();
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

void UCAP_InventoryComponent::ApplyItemStatEffects(UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst || !OwnerASC)
		return;
	UCAP_AbilitySystemComponent* CAP_ASC = Cast<UCAP_AbilitySystemComponent>(OwnerASC);
	if (!CAP_ASC || !CAP_ASC->GetGenerics() || !CAP_ASC->GetGenerics()->GetItemStatInfiniteEffect())
		return;

	UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA();
	if (!ItemDA || ItemDA->GetStatModifiers().IsEmpty())
		return;

	UE_LOG(LogTemp, Warning, TEXT("아이템 보너스 스탯 적용"));
	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(CAP_ASC->GetGenerics()->GetItemStatInfiniteEffect(), 1.f, Context);
	if (SpecHandle.IsValid())
	{
		for (const FGameplayTag& Tag : CachedItemDataTags)
		{
			SpecHandle.Data->SetSetByCallerMagnitude(Tag, 0.f);
		}
		
		for (const TPair<FGameplayTag, float>& StatModifier : ItemDA->GetStatModifiers())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(StatModifier.Key, StatModifier.Value);
		}
		FActiveGameplayEffectHandle ActiveGEHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		ItemStatHandleMap.Add(ItemInst, ActiveGEHandle);
	}
}

void UCAP_InventoryComponent::RemoveItemStatEffects(UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst || !OwnerASC)
		return;
	
	UE_LOG(LogTemp, Warning, TEXT("아이템 보너스 스탯 적용된 것 삭제"));
	if (FActiveGameplayEffectHandle* HandlePtr = ItemStatHandleMap.Find(ItemInst))
	{
		OwnerASC->RemoveActiveGameplayEffect(*HandlePtr);
		ItemStatHandleMap.Remove(ItemInst);
	}
}

void UCAP_InventoryComponent::GiveItemAbility(class UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst || !OwnerASC)
		return;
	UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA();
	if (!ItemDA)
		return;

	UE_LOG(LogTemp, Warning, TEXT("아이템 스킬 활성화"));
	if (UCAP_ItemDataAsset* PassiveItemDA = Cast<UCAP_ItemDataAsset>(ItemDA))
	{
		if (PassiveItemDA->ItemSkills.Num() > 0)
		{
			FGameplayAbilitySpec Spec(UCAP_ItemGameplayAbility::StaticClass(), 1, INDEX_NONE, ItemInst);
			FGameplayAbilitySpecHandle SpecHandle = OwnerASC->GiveAbility(Spec);

			// 즉시 활성화
			OwnerASC->TryActivateAbility(SpecHandle);
			GrantedItemAbilityMap.Add(ItemInst, SpecHandle);
		}
	}
}

void UCAP_InventoryComponent::RemoveItemAbility(class UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst || !OwnerASC)
		return;
	
	UE_LOG(LogTemp, Warning, TEXT("아이템 스킬 제거"));
	if (FGameplayAbilitySpecHandle* HandlePtr = GrantedItemAbilityMap.Find(ItemInst))
	{
		OwnerASC->ClearAbility(*HandlePtr);
		GrantedItemAbilityMap.Remove(ItemInst);
	}
}
