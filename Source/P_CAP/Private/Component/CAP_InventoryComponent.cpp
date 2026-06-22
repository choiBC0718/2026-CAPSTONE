// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interactables/Item/CAP_ItemInstance.h"
#include "GameplayTagsManager.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "Data/CAP_SynergyDataAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "Framework/Subsystem/CAP_SynergySubsystem.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Ability/CAP_ItemGameplayAbility.h"
#include "Interactables/Item/CAP_SynergyInstance.h"
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
	
	FGameplayTag ParentTag = FGameplayTag::RequestGameplayTag(FName("Data.ItemStat"));
	CachedItemDataTags = UGameplayTagsManager::Get().RequestGameplayTagChildren(ParentTag);

	TryRestoreSavedInventory();
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

bool UCAP_InventoryComponent::AddItem(class UCAP_ItemInstance* NewItem)
{
	if (!NewItem) return false;
	
	if (InventoryItems.Num() < Capacity)
	{
		InventoryItems.Add(NewItem);
		OnItemEquipped(NewItem);
		return true;
	}
	
	OnInventoryFull.Broadcast(NewItem);
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

bool UCAP_InventoryComponent::DisassembleItem(class UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst || !ItemInst->GetItemDA())
		return false;

	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwner()))
		if (UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent())
			CurrComp->ProcessDisassembleReward(ItemInst->GetCurrentGrade(),ECurrencyType::Gold);

	RemoveItem(ItemInst);
	return true;
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
	if (!ItemDA)
		return;

	const TArray<FGameplayTag>& Synergies = ItemDA->GetSynergyTags();
	if (Synergies.IsEmpty())
		return;
	
	for (const FGameplayTag& Tag : Synergies)
	{
		int32& Count = CurrentSynergyCounts.FindOrAdd(Tag,0);
		Count += (bIsAdded ? 1 : -1);
		if (Count <=0)
			CurrentSynergyCounts.Remove(Tag);
	}
	RecalculateAllSynergies();
}

void UCAP_InventoryComponent::RecalculateAllSynergies()
{
	if (!OwnerASC)	return;
	UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance())
		return;
	UCAP_SynergySubsystem* SynergySubsystem = World->GetGameInstance()->GetSubsystem<UCAP_SynergySubsystem>();
	if (!SynergySubsystem)
		return;
	
	ClearAllSynergies();
	
	TArray<FSoftObjectPath> PathsToLoad;
	for (const auto& Pair : CurrentSynergyCounts)
	{
		FGameplayTag SynergyTag = Pair.Key;
		int32 Count = Pair.Value;
		if (Count >0 && SynergySubsystem->SynergyMap.Contains(SynergyTag))
		{
			TSoftObjectPtr<UCAP_SynergyDataAsset> SoftPtr = SynergySubsystem->SynergyMap[SynergyTag];
			if (!SoftPtr.IsNull())
				PathsToLoad.AddUnique(SoftPtr.ToSoftObjectPath());
		}
	}

	if (PathsToLoad.Num() > 0)
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		SynergyLoadHandle = Streamable.RequestAsyncLoad(PathsToLoad, FStreamableDelegate::CreateUObject(this, &UCAP_InventoryComponent::OnSynergyDataLoaded));
	}
}

void UCAP_InventoryComponent::ClearAllSynergies()
{
	OwnerASC->RemoveActiveGameplayEffect(SynergyAddMasterHandle);
	OwnerASC->RemoveActiveGameplayEffect(SynergyMulMasterHandle);
	SynergyAddMasterHandle.Invalidate();
	SynergyMulMasterHandle.Invalidate();

	// 스택 유지를 위해 살려두되, 중복 발동을 막기 위해 트리거만 미리 끊어둡니다.
	if (UCAP_AbilitySystemComponent* CASC = Cast<UCAP_AbilitySystemComponent>(OwnerASC))
	{
		for (auto& Pair : ActiveSynergyInstances)
		{
			if (ICAP_BehaviorStateProvider* Provider = Cast<ICAP_BehaviorStateProvider>(Pair.Value))
			{
				for (UCAP_ItemBehaviorBase* Behavior : Provider->GetBehaviors())
				{
					if (Behavior) Behavior->OnUnequipped(Provider, CASC);
				}
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : GrantedSynergyAbilityHandles)
		if (Handle.IsValid())
			OwnerASC->ClearAbility(Handle);
	
	GrantedSynergyAbilityHandles.Empty();
	
	if (SynergyLoadHandle.IsValid() && SynergyLoadHandle->IsActive())
		SynergyLoadHandle->CancelHandle();
}

void UCAP_InventoryComponent::OnSynergyDataLoaded()
{
	if (!OwnerASC) return;
	UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance()) return;
	UCAP_SynergySubsystem* SynergySubsystem = World->GetGameInstance()->GetSubsystem<UCAP_SynergySubsystem>();
	if (!SynergySubsystem) return;
	
	TMap<FGameplayTag, float> AddModifiers;
	TMap<FGameplayTag, float> MulModifiers;

	// 이번 계산에서 하나라도 레벨 조건을 만족하여 살아남은 시너지 태그를 기록합니다.
	TSet<FGameplayTag> ActiveTagsThisFrame;

	for (const auto& Pair : CurrentSynergyCounts)
	{
		FGameplayTag SynTag = Pair.Key;
		int32 Count = Pair.Value;

		if (Count <= 0 || !SynergySubsystem->SynergyMap.Contains(SynTag)) continue;

		UCAP_SynergyDataAsset* SynergyDA = SynergySubsystem->SynergyMap[SynTag].Get();
		if (!SynergyDA) continue;

		TArray<UCAP_ItemBehaviorBase*> BehaviorsToGrant;
		for (const FSynergyLevelData& LvData : SynergyDA->SynergyLevels)
		{
			if (Count >= LvData.RequiredCount)
			{
				for (const FStatModifier& Mod : LvData.StatModifiers)
				{
					if (Mod.IsMultiplier) MulModifiers.FindOrAdd(Mod.StatTag) += Mod.Value;
					else AddModifiers.FindOrAdd(Mod.StatTag) += Mod.Value;
				}
				for (UCAP_ItemBehaviorBase* BehaviorTemplate : LvData.GrantedBehaviors)
				{
					if (BehaviorTemplate) BehaviorsToGrant.AddUnique(BehaviorTemplate);
				}
			}
		}

		if (BehaviorsToGrant.Num() > 0)
		{
			ActiveTagsThisFrame.Add(SynTag); // 이 태그는 활성화 성공

			// 무조건 NewObject를 하지 않고, 기존 객체가 있으면 재활용합니다
			UCAP_SynergyInstance* SynInstance = ActiveSynergyInstances.FindRef(SynTag);
			if (!SynInstance)
			{
				SynInstance = NewObject<UCAP_SynergyInstance>(this);
				ActiveSynergyInstances.Add(SynTag, SynInstance);
			}
			SynInstance->InitializeSynergy(SynTag, Count, BehaviorsToGrant);

			FGameplayAbilitySpec Spec(UCAP_ItemGameplayAbility::StaticClass(), 1, INDEX_NONE, SynInstance);
			FGameplayAbilitySpecHandle Handle = OwnerASC->GiveAbility(Spec);

			if (Handle.IsValid())
			{
				OwnerASC->TryActivateAbility(Handle);
				GrantedSynergyAbilityHandles.Add(Handle);
			}
		}
	}

	// 이번 계산에서 RequiredCount에 도달하지 못해 완전히 꺼져야 하는 시너지 색출 및 즉시 처형
	TArray<FGameplayTag> TagsToKill;
	for (auto& Pair : ActiveSynergyInstances)
	{
		if (!ActiveTagsThisFrame.Contains(Pair.Key))
		{
			TagsToKill.Add(Pair.Key);
		}
	}

	for (const FGameplayTag& DeadTag : TagsToKill)
	{
		UCAP_SynergyInstance* DeadInstance = ActiveSynergyInstances[DeadTag];
		if (DeadInstance)
		{
			// 허공에 남아버린 잔재 GE를 플레이어 몸에서 찾아서 즉각 찢어버림 (즉시 효과 제거)
			FGameplayEffectQuery Query;
			TArray<FActiveGameplayEffectHandle> ActiveEffects = OwnerASC->GetActiveEffects(Query);
			for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
			{
				const FActiveGameplayEffect* ActiveGE = OwnerASC->GetActiveGameplayEffect(Handle);
				if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == DeadInstance)
				{
					OwnerASC->RemoveActiveGameplayEffect(Handle);
				}
			}
			DeadInstance->MarkAsGarbage();
		}
		ActiveSynergyInstances.Remove(DeadTag);
	}

	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	Context.AddSourceObject(this);
	
	ApplyStatModifiers(AddModifiers, false, Context, SynergyAddMasterHandle);
	ApplyStatModifiers(MulModifiers, true, Context, SynergyMulMasterHandle);
}

void UCAP_InventoryComponent::ApplyItemStatEffects(UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst || !OwnerASC)
		return;

	UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA();
	if (!ItemDA || ItemDA->GetStatModifiers().IsEmpty())
		return;

	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	Context.AddSourceObject(ItemInst);

	// 곱연산, 합연산 따로 적용시킬 배열
	TMap<FGameplayTag, float> AddModifiers;
	TMap<FGameplayTag, float> MulModifiers;

	for (const FStatModifier& Mod : ItemDA->GetStatModifiers())
	{
		if (Mod.IsMultiplier)	// 구조체의 IsMultiplier 변수에 따라 배열에 추가
			MulModifiers.FindOrAdd(Mod.StatTag) += Mod.Value;
		else
			AddModifiers.FindOrAdd(Mod.StatTag) += Mod.Value;
	}
	FActiveGameplayEffectHandle AddHandle;
	FActiveGameplayEffectHandle MulHandle;
	ApplyStatModifiers(AddModifiers, false,Context,AddHandle);
	ApplyStatModifiers(MulModifiers, true,Context,MulHandle);

	// 지울수 있도록 캐싱 저장
	TArray<FActiveGameplayEffectHandle> AppliedHandles;
	if (AddHandle.IsValid()) AppliedHandles.Add(AddHandle);
	if (MulHandle.IsValid()) AppliedHandles.Add(MulHandle);
	
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

	if (ICAP_BehaviorStateProvider* Provider = Cast<ICAP_BehaviorStateProvider>(ItemInst))
	{
		if (UCAP_AbilitySystemComponent* CASC = Cast<UCAP_AbilitySystemComponent>(OwnerASC))
		{
			for (UCAP_ItemBehaviorBase* Behavior : Provider->GetBehaviors())
			{
				if (Behavior)
					Behavior->OnUnequipped(Provider, CASC);
			}
		}
	}
	
	if (FGameplayAbilitySpecHandle* HandlePtr = GrantedItemAbilityMap.Find(ItemInst))
	{
		OwnerASC->ClearAbility(*HandlePtr);
		GrantedItemAbilityMap.Remove(ItemInst);
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

void UCAP_InventoryComponent::ApplyStatModifiers(const TMap<FGameplayTag, float>& Modifiers, bool bIsMul,
	const FGameplayEffectContextHandle& Context, FActiveGameplayEffectHandle& OutHandle)
{
	if (Modifiers.IsEmpty() || !OwnerASC)
		return;
	UCAP_AbilitySystemComponent* CASC = Cast<UCAP_AbilitySystemComponent>(OwnerASC);
	if (!CASC || !CASC->GetGenerics())
		return;

	TSubclassOf<UGameplayEffect> TargetGE = CASC->GetGenerics()->GetStatGE(true, bIsMul);
	if (!TargetGE)
		return;

	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(TargetGE, 1.f, Context);
	if (SpecHandle.IsValid())
	{
		for (const FGameplayTag& Tag : CachedItemDataTags)
			SpecHandle.Data->SetSetByCallerMagnitude(Tag, bIsMul ? 1.f : 0.f);
		for (const TPair<FGameplayTag, float>& StatMod : Modifiers)
			SpecHandle.Data->SetSetByCallerMagnitude(StatMod.Key, StatMod.Value);

		OutHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
