// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_ItemDrop.h"

#include "Interactables/Item/CAP_WorldItem.h"

void ANPC_ItemDrop::ExecuteSpecialAction(AActor* Actor)
{
	if (bHasReward || DripItemDataAssets.IsEmpty() || !ItemClass)
		return;

	int32 RandIdx = FMath::RandRange(0,DripItemDataAssets.Num()-1);
	UCAP_ItemDataAsset* SelectedWeaponDA = DripItemDataAssets[RandIdx];

	if (SelectedWeaponDA)
	{
		FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
		FTransform SpawnTrans(GetActorRotation(), SpawnLoc);
		ACAP_WorldItem* SpawnedItem = GetWorld()->SpawnActorDeferred<ACAP_WorldItem>(ItemClass, SpawnTrans);
		if (SpawnedItem)
		{
			SpawnedItem->InitializeItemData(SelectedWeaponDA);
			SpawnedItem->FinishSpawning(SpawnTrans);
		}
	}
	bHasReward = true;
}
