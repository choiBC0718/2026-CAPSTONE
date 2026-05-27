// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_ChestVisualBase.h"
#include "Interactables/CAP_InteractableBase.h"
#include "CAP_RewardChest.generated.h"

UENUM(BlueprintType)
enum class ERewardChestType : uint8
{
	Item,
	Weapon,
	Gold
};

UENUM(BlueprintType)
enum class EChestGrade : uint8
{
	Normal,
	Rare,
	Epic,
	Legendary
};

USTRUCT(BlueprintType)
struct FChestVisualTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERewardChestType ChestType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EChestGrade ChestGrade;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACAP_ChestVisualBase> VisualPrefabClass;
};

USTRUCT(BlueprintType)
struct FDropPoolTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Data")
	class UCAP_ItemDataBase* DropData;
};
/**
 * 
 */
UCLASS()
class ACAP_RewardChest : public ACAP_InteractableBase
{
	GENERATED_BODY()

public:
	ACAP_RewardChest();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Interact(AActor* InsActor, EInteractAction ActionType) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chest")
	ERewardChestType ChestType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chest")
	EChestGrade ChestGrade;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Visuals")
	UDataTable* ChestVisualDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Data")
	TMap<EChestGrade, UDataTable*> ItemDropPoolMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Data")
	TMap<EChestGrade, UDataTable*> WeaponDropPoolMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Data")
	TMap<EChestGrade, int32> GoldSpawnCountMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Data")
	TSubclassOf<class ACAP_InteractableBase> GoldActorClass;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UChildActorComponent* VisualChildActor;
	
protected:
	virtual FInteractionPayload GetInteractionPayload() const override;
	
	bool bIsOpened = false;
	bool bIsCleaningUp = false;
	
	UPROPERTY()
	TArray<AActor*> SpawnedRewardActors;
	
	UFUNCTION()
	void OnRewardItemDestroyed(AActor* DestroyedActor);

	void SpawnReward();
	UCAP_ItemDataBase* GetRandomDropData(UDataTable* TargetDataTable);
	
	void SpawnItem();
	void SpawnWeapon();
	void SpawnGold();
	int32 NumRewardsToSpawn = 0;
};
