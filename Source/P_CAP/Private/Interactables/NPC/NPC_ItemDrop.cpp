// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_ItemDrop.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Interactables/Item/CAP_WorldItem.h"

ENPCActionResult ANPC_ItemDrop::ExecuteSpecialAction(AActor* Actor)
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(Actor);
	if (DripItemDataAssets.IsEmpty() || !ItemClass || !Player)
		return ENPCActionResult::Failed;
	
	if (InteractionCount >= 2)
		return ENPCActionResult::AlreadyReceived;

	if (InteractionCount==1)
	{
		if (UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent())
		{
			if (!CurrComp->ConsumeCurrency(ECurrencyType::MagicStone, CostMagicStone))
				return ENPCActionResult::InsufficientCurrency;
		}
	}
	
	int32 RandIdx = FMath::RandRange(0,DripItemDataAssets.Num()-1);
	UCAP_ItemDataAsset* SelectedItemDA = DripItemDataAssets[RandIdx];

	if (SelectedItemDA)
	{
		FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
		FTransform SpawnTrans(GetActorRotation(), SpawnLoc);
		ACAP_WorldItem* SpawnedItem = GetWorld()->SpawnActorDeferred<ACAP_WorldItem>(ItemClass, SpawnTrans);
		if (SpawnedItem)
		{
			SpawnedItem->InitializeItemData(SelectedItemDA);
			SpawnedItem->FinishSpawning(SpawnTrans);
		}
		InteractionCount++;
		return ENPCActionResult::Success;
	}
	return ENPCActionResult::Failed;
}

bool ANPC_ItemDrop::GetSpecialActionCost(AActor* Actor, int32& OutCost)
{
	if (InteractionCount == 1)
	{
		OutCost = CostMagicStone;
		return true;
	}
	return false;
}
