// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Reward/RewardWeaponSpawner.h"

#include "Data/CAP_WeaponDataAsset.h"
#include "Map/SpecialRoomTransitionSubsystem.h"
#include "Interactables/Weapon/CAP_WorldWeapon.h"

ARewardWeaponSpawner::ARewardWeaponSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	WeaponClass = ACAP_WorldWeapon::StaticClass();
}

void ARewardWeaponSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnRandomWeapon();
	}
}

ACAP_WorldWeapon* ARewardWeaponSpawner::SpawnRandomWeapon()
{
	if (bSpawnOnlyOnce && IsValid(SpawnedWeapon))
	{
		return SpawnedWeapon.Get();
	}

	if (!WeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("RewardWeaponSpawner: WeaponClass is not set on %s."), *GetName());
		return nullptr;
	}

	USpecialRoomTransitionSubsystem* TransitionSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<USpecialRoomTransitionSubsystem>()
		: nullptr;
	FIntPoint SpecialRoomGridPos = FIntPoint::ZeroValue;
	const bool bHasSpecialRoomGridPos = TransitionSubsystem &&
		TransitionSubsystem->TryGetPendingSpecialRoomGridPos(SpecialRoomGridPos);
	if (bConsumeSpecialRoomRewardOnSpawn &&
		bHasSpecialRoomGridPos &&
		TransitionSubsystem->IsSpecialRoomRewardConsumed(SpecialRoomGridPos))
	{
		return nullptr;
	}

	TArray<UCAP_WeaponDataAsset*> ValidWeaponPool;
	ValidWeaponPool.Reserve(WeaponPool.Num());
	for (UCAP_WeaponDataAsset* WeaponDataAsset : WeaponPool)
	{
		if (WeaponDataAsset)
		{
			ValidWeaponPool.Add(WeaponDataAsset);
		}
	}

	if (ValidWeaponPool.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("RewardWeaponSpawner: WeaponPool is empty on %s."), *GetName());
		return nullptr;
	}

	UCAP_WeaponDataAsset* SelectedWeaponDataAsset = bHasSpecialRoomGridPos
		? TransitionSubsystem->GetSpecialRoomRewardWeapon(SpecialRoomGridPos)
		: nullptr;

	if (!SelectedWeaponDataAsset)
	{
		SelectedWeaponDataAsset = ValidWeaponPool[FMath::RandRange(0, ValidWeaponPool.Num() - 1)];
		if (bHasSpecialRoomGridPos)
		{
			TransitionSubsystem->SetSpecialRoomRewardWeapon(SpecialRoomGridPos, SelectedWeaponDataAsset);
		}
	}

	if (!SelectedWeaponDataAsset)
	{
		return nullptr;
	}

	const FTransform SpawnTransform(GetActorRotation(), GetActorLocation() + SpawnOffset);
	ACAP_WorldWeapon* NewWeapon = GetWorld()->SpawnActorDeferred<ACAP_WorldWeapon>(WeaponClass, SpawnTransform);
	if (!NewWeapon)
	{
		return nullptr;
	}

	NewWeapon->InitializeWeaponData(SelectedWeaponDataAsset);
	NewWeapon->FinishSpawning(SpawnTransform);
	SpawnedWeapon = NewWeapon;

	if (bConsumeSpecialRoomRewardOnSpawn && bHasSpecialRoomGridPos)
	{
		NewWeapon->SetSpecialRoomRewardSource(SpecialRoomGridPos);
	}

	return NewWeapon;
}
