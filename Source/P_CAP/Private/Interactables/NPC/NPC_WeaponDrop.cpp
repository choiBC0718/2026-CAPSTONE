// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_WeaponDrop.h"

#include "Interactables/Weapon/CAP_WorldWeapon.h"

void ANPC_WeaponDrop::ExecuteSpecialAction(AActor* Actor)
{
	if (bHasReward || DropWeaponDataAssets.IsEmpty() || !WeaponClass)
		return;

	int32 RandIdx = FMath::RandRange(0,DropWeaponDataAssets.Num()-1);
	UCAP_WeaponDataAsset* SelectedWeaponDA = DropWeaponDataAssets[RandIdx];

	if (SelectedWeaponDA)
	{
		FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
		FTransform SpawnTrans(GetActorRotation(), SpawnLoc);
		ACAP_WorldWeapon* SpawnedWeapon = GetWorld()->SpawnActorDeferred<ACAP_WorldWeapon>(WeaponClass, SpawnTrans);
		if (SpawnedWeapon)
		{
			SpawnedWeapon->InitializeWeaponData(SelectedWeaponDA);
			SpawnedWeapon->FinishSpawning(SpawnTrans);
		}
	}
	bHasReward = true;
}
