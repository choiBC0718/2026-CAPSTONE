// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Data/CAP_SynergyTypes.h"
#include "CAP_InventoryComponent.generated.h"

// 인벤토리 변경 알림 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, class UCAP_ItemInstance*, ChangedItem, bool, bIsAdded);
// 인벤토리 가득 참 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryFull, class UCAP_ItemInstance*, OverflowItem);

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

	void RefreshSynergies();
	bool SwapItem(class UCAP_ItemInstance* OldItem, class UCAP_ItemInstance* NewItem);

	const TMap<FGameplayTag, int32> & GetCurrentSynergyCounts() const {return CurrentSynergyCounts;}
	const TMap<FGameplayTag, FSynergyDataTable*>& GetSynergyDataCache() const {return SynergyDataCache;}
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	int Capacity = 9;

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
	
	UPROPERTY(EditDefaultsOnly, Category="Synergy")
	UDataTable* SynergyDataTable;

	// 데이터 테이블 캐시
	TMap<FGameplayTag, FSynergyDataTable*> SynergyDataCache;
	
	void UpdateSynergyEffects();
};
