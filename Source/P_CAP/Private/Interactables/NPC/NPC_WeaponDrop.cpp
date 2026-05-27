// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_WeaponDrop.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Interactables/Weapon/CAP_WorldWeapon.h"

ENPCActionResult ANPC_WeaponDrop::ExecuteSpecialAction(AActor* Actor)
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(Actor);
	if (!Player || DropWeaponDataAssets.IsEmpty() || !WeaponClass)
		return ENPCActionResult::Failed;
	
	if (InteractionCount >= 2)
		return ENPCActionResult::AlreadyReceived;

	TArray<UCAP_WeaponDataAsset*> ValidDrops;
	UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent();
	for (UCAP_WeaponDataAsset* DA : DropWeaponDataAssets)
	{
		if (!DA)	continue;
		if (InteractionCount>0 && DA==FirstDroppedWeapon)	continue;
		if (WeaponComp && WeaponComp->HasWeapon(DA))	continue;
		ValidDrops.Add(DA);
	}
	if (ValidDrops.IsEmpty())
		return ENPCActionResult::Failed;

	if (InteractionCount == 0)
	{
		if (!bIsActionPending)
		{
			bIsActionPending = true;
			return ENPCActionResult::FirstInteraction;
		}
		bIsActionPending = false;
		
		int32 RandIdx = FMath::RandRange(0,ValidDrops.Num()-1);
		if (UCAP_WeaponDataAsset* SelectedWeaponDA = ValidDrops[RandIdx])
		{
			FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 150.f;
			FTransform SpawnTrans(GetActorRotation(), SpawnLoc);
			ACAP_WorldWeapon* SpawnedWeapon = GetWorld()->SpawnActorDeferred<ACAP_WorldWeapon>(WeaponClass, SpawnTrans);
			if (SpawnedWeapon)
			{
				SpawnedWeapon->InitializeWeaponData(SelectedWeaponDA);
				SpawnedWeapon->FinishSpawning(SpawnTrans);
			}
			FirstDroppedWeapon = SelectedWeaponDA;
			InteractionCount++;
			
			return ENPCActionResult::Success;
		}
		return ENPCActionResult::Failed;
	}
	
	if (InteractionCount==1)
	{
		if (!bIsPaymentPending)
		{
			bIsPaymentPending = true;
			return ENPCActionResult::RequireConfirm;
		}
		bIsPaymentPending = false;
		
		// 재화 검사 및 소모
		if (UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent())
		{
			if (!CurrComp->ConsumeCurrency(ECurrencyType::MagicStone, CostMagicStone))
				return ENPCActionResult::InsufficientCurrency;
		}
		else
			return ENPCActionResult::Failed;

		int32 RandIdx = FMath::RandRange(0, ValidDrops.Num() - 1);
		UCAP_WeaponDataAsset* SelectedWeaponDA = ValidDrops[RandIdx];

		if (SelectedWeaponDA)
		{
			FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 150.f;
			FTransform SpawnTrans(GetActorRotation(), SpawnLoc);
			ACAP_WorldWeapon* SpawnedWeapon = GetWorld()->SpawnActorDeferred<ACAP_WorldWeapon>(WeaponClass, SpawnTrans);
			if (SpawnedWeapon)
			{
				SpawnedWeapon->InitializeWeaponData(SelectedWeaponDA);
				SpawnedWeapon->FinishSpawning(SpawnTrans);
			}
			
			InteractionCount++;
			return ENPCActionResult::Success;
		}
	}
	return ENPCActionResult::Failed;
}