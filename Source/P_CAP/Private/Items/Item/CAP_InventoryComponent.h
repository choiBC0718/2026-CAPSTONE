// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "CAP_InventoryComponent.generated.h"

// 인벤토리 변경 알림 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, class UCAP_ItemInstance*, ChangedItem, bool, bIsAdded);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCAP_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCAP_InventoryComponent();

	virtual void BeginPlay() override;
	FORCEINLINE int GetCapacity() const { return Capacity; }

	bool AddItem(class UCAP_ItemInstance* NewItem);
	const TArray<class UCAP_ItemInstance*> GetInventoryItems() const {return InventoryItems;}
	
	UPROPERTY()
	FOnInventoryChanged OnInventoryChanged;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	int Capacity = 9;

	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;
	UPROPERTY()
	TArray<class UCAP_ItemInstance*> InventoryItems;
};
