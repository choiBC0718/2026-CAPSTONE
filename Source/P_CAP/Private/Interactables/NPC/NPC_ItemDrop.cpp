// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_ItemDrop.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Interactables/Item/CAP_WorldItem.h"

ENPCActionResult ANPC_ItemDrop::ExecuteSpecialAction(AActor* Actor)
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(Actor);
	if (!Player || DropItemDataAssets.IsEmpty() || !ItemClass)
		return ENPCActionResult::Failed;
	
	if (InteractionCount >= 2)
		return ENPCActionResult::AlreadyReceived;

	if (InteractionCount == 0)
	{
		if (!bIsActionPending)
		{
			bIsActionPending = true;
			return ENPCActionResult::FirstInteraction;
		}
		
		bIsActionPending = false;
		int32 RandIdx = FMath::RandRange(0, DropItemDataAssets.Num() - 1);
		UCAP_ItemDataAsset* SelectedItemDA = DropItemDataAssets[RandIdx];

		if (SelectedItemDA)
		{
			FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 150.f;
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

	if (InteractionCount == 1)
	{
		if (!bIsPaymentPending)
		{
			bIsPaymentPending = true;
			return ENPCActionResult::RequireConfirm;
		}
		
		bIsPaymentPending = false;

		if (UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent())
		{
			if (!CurrComp->ConsumeCurrency(ECurrencyType::MagicStone, CostMagicStone))
				return ENPCActionResult::InsufficientCurrency;
		}
	
		int32 RandIdx = FMath::RandRange(0, DropItemDataAssets.Num() - 1);
		UCAP_ItemDataAsset* SelectedItemDA = DropItemDataAssets[RandIdx];

		if (SelectedItemDA)
		{
			FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 150.f;
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
	}

	return ENPCActionResult::Failed;
}
