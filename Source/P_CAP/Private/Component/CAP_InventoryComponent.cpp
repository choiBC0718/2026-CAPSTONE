// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interactables/Item/CAP_ItemInstance.h"
#include "GameplayTagsManager.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Ability/CAP_ItemGameplayAbility.h"
#include "Kismet/GameplayStatics.h"


UCAP_InventoryComponent::UCAP_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCAP_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwner()))
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

	TryRestoreSavedInventory();
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
	
	OnItemEquipped(NewItem);
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
		OnItemUnequipped(OldItem);
		
		//UE_LOG(LogTemp, Warning, TEXT("새로운 아이템 효과 적용"));
		InventoryItems[Index] = NewItem;
		OnItemEquipped(NewItem);
		return true;
	}
	return false;
}

void UCAP_InventoryComponent::RemoveItem(class UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst) return;

	OnItemUnequipped(ItemInst);
	InventoryItems.Remove(ItemInst);
	
	if (OnInventoryChanged.IsBound())
		OnInventoryChanged.Broadcast(ItemInst, false);
}

struct FInventorySaveData UCAP_InventoryComponent::CreateSaveData() const
{
	FInventorySaveData SaveData;
	for (UCAP_ItemInstance* Item : InventoryItems)
	{
		if (Item && Item->GetItemDA())
			SaveData.HeldItemDAs.Add(Item->GetItemDA());
	}
	return SaveData;
}

void UCAP_InventoryComponent::UpdateItemSynergies(class UCAP_ItemDataBase* ItemDA, bool bIsAdded)
{
	if (!ItemDA || SynergyDataCache.IsEmpty())
		return;

	const TArray<FGameplayTag>& Synergies = ItemDA->GetSynergyTags();
	for (const FGameplayTag& Tag : Synergies)
	{
		int32& Count = CurrentSynergyCounts.FindOrAdd(Tag,0);
		Count += (bIsAdded ? 1 : -1);
		UpdateSingleSynergyEffect(Tag, Count);
	}
}

void UCAP_InventoryComponent::UpdateSingleSynergyEffect(FGameplayTag Tag, int32 NewCount)
{
	if (!OwnerASC)
		return;
	int32 LastCount = CachedSynergyCounts.Contains(Tag) ? CachedSynergyCounts[Tag] : 0;
	if (NewCount == LastCount)
		return;

	ClearSynergyEffects(Tag);
	if (NewCount <= 0)
	{
		CachedSynergyCounts.Remove(Tag);
		AppliedSynergyHandles.Remove(Tag);
		return;
	}

	if (FSynergyDataTable** RowPtr = SynergyDataCache.Find(Tag))
	{
		FSynergyDataTable* Row = *RowPtr;
		for (const FSynergyLevelData& SynergyLevel : Row->SynergyLevels)
		{
			if (NewCount>=SynergyLevel.RequiredCount)
				ApplySynergyEffect(Tag, SynergyLevel.SynergyEffect);
		}
	}
	CachedSynergyCounts.Add(Tag, NewCount);
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
				SpecHandle.Data->SetSetByCallerMagnitude(Tag, bIsMul ? 1.f : 0.f);
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
	
	if (FGameplayAbilitySpecHandle* HandlePtr = GrantedItemAbilityMap.Find(ItemInst))
	{
		OwnerASC->ClearAbility(*HandlePtr);
		GrantedItemAbilityMap.Remove(ItemInst);
	}
}

void UCAP_InventoryComponent::ClearSynergyEffects(FGameplayTag Tag)
{
	if (!Tag.IsValid())	return;

	if (AppliedSynergyHandles.Contains(Tag))
	{
		for (FActiveGameplayEffectHandle& Handle : AppliedSynergyHandles[Tag])
		{
			OwnerASC->RemoveActiveGameplayEffect(Handle);
		}
		AppliedSynergyHandles[Tag].Empty();
	}
	else
		AppliedSynergyHandles.Add(Tag, TArray<FActiveGameplayEffectHandle>());
}

void UCAP_InventoryComponent::ApplySynergyEffect(FGameplayTag Tag, TSubclassOf<UGameplayEffect> GE)
{
	if (!Tag.IsValid() || !GE)
		return;

	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(GE, 1.f, Context);
	if (Spec.IsValid())
	{
		FActiveGameplayEffectHandle ActiveHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		AppliedSynergyHandles[Tag].Add(ActiveHandle);
	}
}

void UCAP_InventoryComponent::TryRestoreSavedInventory()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UCAP_ProgressionSubsystem* Subsys = GI->GetSubsystem<UCAP_ProgressionSubsystem>())
		{
			FPlayerProgressionData SavedData;
			if (Subsys->LoadPlayerProgression(SavedData) && SavedData.bIsValid)
			{
				RestoreFromSaveData(SavedData.InventoryData);
			}
		}
	}
}

void UCAP_InventoryComponent::RestoreFromSaveData(const struct FInventorySaveData& InData)
{
	// 초기화
	for (UCAP_ItemInstance* Item : InventoryItems)
		if (Item)
			OnItemUnequipped(Item);
	InventoryItems.Empty();

	for (UCAP_ItemDataBase* ItemDA : InData.HeldItemDAs)
		if (ItemDA)
		{
			UCAP_ItemInstance* NewItem = NewObject<UCAP_ItemInstance>(this);
			NewItem->Initialize(ItemDA);
			AddItem(NewItem);
		}
}

void UCAP_InventoryComponent::OnItemEquipped(UCAP_ItemInstance* ItemInst)
{
	ApplyItemStatEffects(ItemInst);
	GiveItemAbility(ItemInst);
	UpdateItemSynergies(ItemInst->GetItemDA(), true);
	if (OnInventoryChanged.IsBound())
		OnInventoryChanged.Broadcast(ItemInst,true);
}

void UCAP_InventoryComponent::OnItemUnequipped(UCAP_ItemInstance* ItemInst)
{
	RemoveItemStatEffects(ItemInst);
	RemoveItemAbility(ItemInst);
	UpdateItemSynergies(ItemInst->GetItemDA(), false);
}
