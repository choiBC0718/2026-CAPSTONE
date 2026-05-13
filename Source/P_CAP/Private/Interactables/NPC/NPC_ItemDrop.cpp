// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_ItemDrop.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Interactables/Item/CAP_WorldItem.h"

ENPCActionResult ANPC_ItemDrop::ExecuteSpecialAction(AActor* Actor)
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(Actor);
	UE_LOG(LogTemp,Warning,TEXT("스페셜 액션 시작"));
	if (DripItemDataAssets.IsEmpty() || !ItemClass || !Player)
	{
		UE_LOG(LogTemp,Warning,TEXT("비어있거나 클래스 없거나 플레이어 없어서 실패"));
		return ENPCActionResult::Failed;
	}
	
	if (InteractionCount >= 2)
	{
		UE_LOG(LogTemp,Warning,TEXT("이미 두번 받아감"));
		return ENPCActionResult::AlreadyReceived;
	}

	if (InteractionCount==1)
	{
		if (UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent())
		{
			if (!CurrComp->ConsumeCurrency(ECurrencyType::MagicStone, CostMagicStone))
			{
				UE_LOG(LogTemp,Warning,TEXT("재화 부족함"));
				return ENPCActionResult::InsufficientCurrency;
			}
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
		UE_LOG(LogTemp,Warning,TEXT("오케이 통과"));
		return ENPCActionResult::Success;
	}
	UE_LOG(LogTemp,Warning,TEXT("그냥 최종 결과가 실패임"));
	return ENPCActionResult::Failed;
}
