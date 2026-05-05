// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "CAP_InventoryComponent.generated.h"

// 인벤토리 변경 알림 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, class UCAP_ItemInstance*, ChangedItem, bool, bIsAdded);
// 인벤토리 가득 참 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryFull, class UCAP_ItemInstance*, OverflowItem);
// 아이템 효과 발동 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnItemEffectTriggered, class UCAP_ItemInstance*, ItemInst, FGameplayTag, DynamicTag, float,Cooldown,float,Duration,int32,Stacks);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCAP_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCAP_InventoryComponent();
	virtual void BeginPlay() override;
	
	FORCEINLINE int GetCapacity() const { return Capacity; }

	bool AddItem(class UCAP_ItemInstance* NewItem);
	const TArray<class UCAP_ItemInstance*>& GetInventoryItems() const {return InventoryItems;}
	
	UPROPERTY()
	FOnInventoryChanged OnInventoryChanged;
	UPROPERTY()
	FOnInventoryFull OnInventoryFull;
	UPROPERTY()
	FOnItemEffectTriggered OnItemEffectTriggered;
	
	bool SwapItem(class UCAP_ItemInstance* OldItem, class UCAP_ItemInstance* NewItem);
	void RemoveItemEffect(class UCAP_ItemInstance* ItemInst);
	void RemoveItem(class UCAP_ItemInstance* ItemInst);

	const TMap<FGameplayTag, int32> & GetCurrentSynergyCounts() const {return CurrentSynergyCounts;}
	const TMap<FGameplayTag, FSynergyDataTable*>& GetSynergyDataCache() const {return SynergyDataCache;}

private:
	// 아이템 장착 가능 수
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	int Capacity = 9;
	// 참조할 시너지 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category="Synergy")
	UDataTable* SynergyDataTable;
	// 아이템 장착 최대 누름 지속 시간
	
	UPROPERTY()
	class ACAP_PlayerCharacter* Player;
	UPROPERTY()
	class UAbilitySystemComponent* OwnerASC;
	UPROPERTY()
	TArray<class UCAP_ItemInstance*> InventoryItems;
	
	// 현재 모인 시너지 개수
	TMap<FGameplayTag, int32> CurrentSynergyCounts;
	// 저장됐던 태그에 대한 개수
	TMap<FGameplayTag, int32> CachedSynergyCounts;
	// 현재 캐릭터에게 적용된 시너지 버프
	TMap<FGameplayTag, TArray<FActiveGameplayEffectHandle>> AppliedSynergyHandles;

	// 데이터 테이블 캐시
	TMap<FGameplayTag, FSynergyDataTable*> SynergyDataCache;

	// 시너지 재계산
	void RefreshSynergies();
	// 시너지 효과 재계산
	void UpdateSynergyEffects();
	
	FGameplayTagContainer CachedItemDataTags;

	// 아이템의 효과를 저장해 놓을 Map <Key: 아이템 정보 || Value: 아이템 효과>
	UPROPERTY()
	TMap<class UCAP_ItemInstance*, FActiveGameplayEffectHandle> ItemStatHandleMap;
	// 아이템의 스킬 저장해놓을 Map <Key: 아이템 정보 || Value: 아이템 스킬>
	UPROPERTY()
	TMap<class UCAP_ItemInstance*, FGameplayAbilitySpecHandle> GrantedItemAbilityMap;

	// 아이템 장착 시 아이템이 제공하는 보너스 스탯 적용
	void ApplyItemStatEffects(UCAP_ItemInstance* ItemInst);
	// 아이템 해제 시 보너스 스탯 제거
	void RemoveItemStatEffects(UCAP_ItemInstance* ItemInst);
	// 아이템 스킬 장착
	void GiveItemAbility(class UCAP_ItemInstance* ItemInst);
	// 아이템 스킬 해제
	void RemoveItemAbility(class UCAP_ItemInstance* ItemInst);
};
