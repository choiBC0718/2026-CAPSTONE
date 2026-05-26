// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interactables/Item/CAP_ItemInstance.h"
#include "GameplayTagsManager.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Ability/CAP_ItemGameplayAbility.h"


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
	
	int32 TargetIndex = INDEX_NONE;
	for (int32 i = 0; i < InventoryItems.Num(); ++i)
	{
		if (InventoryItems[i]==nullptr)
		{
			TargetIndex = i;
			break;
		}
	}
	if (TargetIndex == INDEX_NONE && InventoryItems.Num() >= Capacity)
	{
		OnInventoryFull.Broadcast(NewItem);
		return false;
	}
	
	if (TargetIndex != INDEX_NONE)
		InventoryItems[TargetIndex] = NewItem;
	else
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
		//UE_LOG(LogTemp, Warning, TEXT("아이템 스왑 로직 - 적용되어 있던 아이템 제거(효과 제거)"));
		RemoveItemEffect(OldItem);
		
		//UE_LOG(LogTemp, Warning, TEXT("새로운 아이템 효과 적용"));
		InventoryItems[Index] = NewItem;
		ApplyItemStatEffects(NewItem);
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
	//UE_LOG(LogTemp,Warning, TEXT("인벤토리에서 아이템 제거"));
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
	if (!CAP_ASC || !CAP_ASC->GetGenerics())
		return;

	UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA();
	if (!ItemDA || ItemDA->GetStatModifiers().IsEmpty())
		return;

	//UE_LOG(LogTemp, Warning, TEXT("아이템 보너스 스탯 적용"));
	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	Context.AddSourceObject(ItemInst);

	// 곱연산, 합연산 따로 적용시킬 배열
	TMap<FGameplayTag, float> AddModifiers;
	TMap<FGameplayTag, float> MulModifiers;

	for (const FStatModifier& Mod : ItemDA->GetStatModifiers())
	{
		if (Mod.IsMultiplier)	// 구조체의 IsMultiplier 변수에 따라 배열에 추가
			MulModifiers.Add(Mod.StatTag, Mod.Value);
		else
			AddModifiers.Add(Mod.StatTag, Mod.Value);
	}
	// 최종 적용된 버프를 저장할 배열
	TArray<FActiveGameplayEffectHandle> AppliedHandles;
	
	auto ApplyModifiers = [&](const TMap<FGameplayTag, float>& Modifiers, bool bIsMul)
	{
		if (Modifiers.IsEmpty())	return;
		
		TSubclassOf<UGameplayEffect> TargetGE = CAP_ASC->GetGenerics()->GetStatGE(true,bIsMul);
		if (!TargetGE)	return;

		FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(TargetGE, 1.f, Context);
		if (SpecHandle.IsValid())
		{
			// 에디터 에러 로그 방지용 모든 태그에 대해 0으로 초기화
			for (const FGameplayTag& Tag : CachedItemDataTags)
				SpecHandle.Data->SetSetByCallerMagnitude(Tag, 0.f);
			// 실제 설정한 값에는 정상적인 값 전달
			for (const TPair<FGameplayTag, float>& StatMod : Modifiers)
				SpecHandle.Data->SetSetByCallerMagnitude(StatMod.Key, StatMod.Value);
			// 자신에게 Spec 적용 후, 적용된 버프 배열에 추가
			FActiveGameplayEffectHandle ActiveHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			AppliedHandles.Add(ActiveHandle);
		}
	};
	// 곱연산, 합연산에 따라 다르게 람다 함수 적용
	ApplyModifiers(AddModifiers, false);
	ApplyModifiers(MulModifiers, true);

	if (AppliedHandles.Num() > 0)
	{	// 구조체로 감싼 후 Map에 저장
		FActiveGEHandleArray HandleWrapper;
		HandleWrapper.Handles = AppliedHandles;
		ItemStatHandleMap.Add(ItemInst, HandleWrapper);
	}
}

void UCAP_InventoryComponent::RemoveItemStatEffects(UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst || !OwnerASC)
		return;
	
	//UE_LOG(LogTemp, Warning, TEXT("아이템 보너스 스탯 적용된 것 삭제"));
	if (FActiveGEHandleArray* HandlesWrapper = ItemStatHandleMap.Find(ItemInst))
	{
		for (const FActiveGameplayEffectHandle& Handle : HandlesWrapper->Handles)
			OwnerASC->RemoveActiveGameplayEffect(Handle);
	}
	ItemStatHandleMap.Remove(ItemInst);
}

void UCAP_InventoryComponent::GiveItemAbility(class UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst || !OwnerASC)
		return;
	UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA();
	if (!ItemDA)
		return;

	//UE_LOG(LogTemp, Warning, TEXT("아이템 스킬 활성화"));
	if (UCAP_ItemDataAsset* PassiveItemDA = Cast<UCAP_ItemDataAsset>(ItemDA))
	{
		if (PassiveItemDA->ItemBehaviors.Num() > 0)
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
	
	//UE_LOG(LogTemp, Warning, TEXT("아이템 스킬 제거"));
	if (FGameplayAbilitySpecHandle* HandlePtr = GrantedItemAbilityMap.Find(ItemInst))
	{
		OwnerASC->ClearAbility(*HandlePtr);
		GrantedItemAbilityMap.Remove(ItemInst);
	}
}
