// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Reward/CAP_RewardChest.h"

#include "Components/SphereComponent.h"
#include "Interactables/Item/CAP_WorldItem.h"
#include "Interactables/Weapon/CAP_WorldWeapon.h"

ACAP_RewardChest::ACAP_RewardChest()
{
	bIsOpened = false;
	bIsCleaningUp = false;
	ChestType = ERewardChestType::Item;
	ChestGrade = EChestGrade::Normal;

	VisualChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("VisualChildActor"));
	VisualChildActor->SetupAttachment(InteractionSphere);

	GoldSpawnCountMap.Add(EChestGrade::Normal, 1);
	GoldSpawnCountMap.Add(EChestGrade::Rare, 3);
	GoldSpawnCountMap.Add(EChestGrade::Epic, 5);
	GoldSpawnCountMap.Add(EChestGrade::Legendary, 10);
}

void ACAP_RewardChest::BeginPlay()
{
	Super::BeginPlay();
}

void ACAP_RewardChest::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ChestVisualDataTable)
	{
		TArray<FChestVisualTableRow*> AllVisualRows;
		ChestVisualDataTable->GetAllRows<FChestVisualTableRow>("", AllVisualRows);

		for (FChestVisualTableRow* Row : AllVisualRows)
		{
			if (Row && Row->ChestType == ChestType && Row->ChestGrade == ChestGrade)
			{
				if (VisualChildActor && Row->VisualPrefabClass)
				{
					VisualChildActor->SetChildActorClass(Row->VisualPrefabClass);
				}
				break;
			}
		}
	}
}

void ACAP_RewardChest::Interact(AActor* InsActor, EInteractAction ActionType)
{
	if (bIsOpened)
		return;
	bIsOpened = true;
	if (ACAP_ChestVisualBase* VisualBase = Cast<ACAP_ChestVisualBase>(VisualChildActor->GetChildActor()))
	{
		VisualBase->PlayOpenAnim();
	}
	SpawnReward();
}

UCAP_ItemDataBase* ACAP_RewardChest::GetRandomDropData(UDataTable* TargetDataTable)
{
	if (!TargetDataTable)
		return nullptr;

	TArray<FName> RowNames = TargetDataTable->GetRowNames();
	if (RowNames.Num() == 0)
		return nullptr;

	int32 RandIdx = FMath::RandRange(0, RowNames.Num() - 1);
	FName SelectedName = RowNames[RandIdx];

	FDropPoolTableRow* DropRow = TargetDataTable->FindRow<FDropPoolTableRow>(SelectedName,"");
	if (DropRow)
		return DropRow->DropData;
	return nullptr;
}


FInteractionPayload ACAP_RewardChest::GetInteractionPayload() const
{
	// 1회 상호작용 하고 나면 전달값 없어야함
	FInteractionPayload Payload;
	Payload.ActionData.ShortActionText = TEXT("살펴보기");
	return Payload;
}

void ACAP_RewardChest::OnRewardItemDestroyed(AActor* DestroyedActor)
{
	if (bIsCleaningUp)
		return;
	
	bIsCleaningUp = true;
	
	for (AActor* RemainingActor : SpawnedRewardActors)
	{
		if (RemainingActor && RemainingActor != DestroyedActor && IsValid(RemainingActor))
		{
			// 남은 아이템 파괴 이펙트
			RemainingActor->Destroy();
		}
	}
	SpawnedRewardActors.Empty();
}

void ACAP_RewardChest::SpawnReward()
{
	switch (ChestType)
	{
	case ERewardChestType::Item:
		SpawnItem();
		break;
	case ERewardChestType::Weapon:
		SpawnWeapon();
		break;
	case ERewardChestType::Gold:
		SpawnGold();
		break;
	}
}

void ACAP_RewardChest::SpawnItem()
{
	NumRewardsToSpawn = 4;
	UDataTable* TargetDT=nullptr;
	if (ItemDropPoolMap.Contains(ChestGrade))
		TargetDT = ItemDropPoolMap[ChestGrade];

	TArray<UCAP_ItemDataBase*> SelectedItemDAs;
	if (TargetDT)
	{
		TArray<FName> RowNames = TargetDT->GetRowNames();
		int32 ActualSpawnCount = FMath::Min(NumRewardsToSpawn, RowNames.Num());

		if (ActualSpawnCount > 0)
		{
			for (int32 i = RowNames.Num() - 1; i > 0; --i)
			{
				int32 SwapIndex = FMath::RandRange(0, i);
				RowNames.Swap(i, SwapIndex);
			}

			for (int32 i = 0; i < ActualSpawnCount; ++i)
			{
				FDropPoolTableRow* DropRow = TargetDT->FindRow<FDropPoolTableRow>(RowNames[i], "");
				if (DropRow && DropRow->DropData)
				{
					SelectedItemDAs.Add(DropRow->DropData);
				}
			}
		}
	}
	
	float Radius = 250.0f;
	for (int32 i=0;i<SelectedItemDAs.Num();++i)
	{
		float AngleDegrees = i * (360.0f / NumRewardsToSpawn);
		float AngleRadians = FMath::DegreesToRadians(AngleDegrees);
		
		FVector Offset(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 50.0f);
		FVector SpawnLoc = GetActorLocation() + Offset;
		FRotator SpawnRot = FRotator::ZeroRotator;
		FTransform SpawnTrans(SpawnRot, SpawnLoc);
		
		if (ACAP_WorldItem* WorldItem = GetWorld()->SpawnActorDeferred<ACAP_WorldItem>(ACAP_WorldItem::StaticClass(), SpawnTrans))
		{
			WorldItem->InitializeItemData(Cast<UCAP_ItemDataAsset>(SelectedItemDAs[i]));
			WorldItem->FinishSpawning(SpawnTrans);
			WorldItem->DropItem();
			WorldItem->OnDestroyed.AddDynamic(this, &ACAP_RewardChest::OnRewardItemDestroyed);
				
			SpawnedRewardActors.Add(WorldItem);
		}
	}
}

void ACAP_RewardChest::SpawnWeapon()
{
	NumRewardsToSpawn = 1;
	UDataTable* TargetDT=nullptr;
	if (WeaponDropPoolMap.Contains(ChestGrade)) 
		TargetDT = WeaponDropPoolMap[ChestGrade];

	FVector SpawnLoc = GetActorLocation();
	FRotator SpawnRot = FRotator::ZeroRotator;
	FTransform SpawnTrans(SpawnRot, SpawnLoc);

	if (TargetDT)
	{
		if (UCAP_ItemDataBase* RandomDA = GetRandomDropData(TargetDT))
		{
			if (ACAP_WorldWeapon* WorldWeapon = GetWorld()->SpawnActorDeferred<ACAP_WorldWeapon>(ACAP_WorldWeapon::StaticClass(), SpawnTrans))
			{
				WorldWeapon->InitializeWeaponData(Cast<UCAP_WeaponDataAsset>(RandomDA));
				WorldWeapon->FinishSpawning(SpawnTrans);
				WorldWeapon->DropItem();
				WorldWeapon->OnDestroyed.AddDynamic(this, &ACAP_RewardChest::OnRewardItemDestroyed);
			}
		}
	}
}

void ACAP_RewardChest::SpawnGold()
{
	if (GoldSpawnCountMap.Contains(ChestGrade))
		NumRewardsToSpawn=GoldSpawnCountMap[ChestGrade];

	for (int32 i = 0; i < NumRewardsToSpawn; ++i)
	{
		FVector SpawnLoc = GetActorLocation()+FVector(FMath::RandRange(-150.f, 150.f), FMath::RandRange(-150.f, 150.f), 50.f);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		if (GoldActorClass)
			GetWorld()->SpawnActor<ACAP_InteractableBase>(GoldActorClass, SpawnLoc, SpawnRotation);
	}
}
