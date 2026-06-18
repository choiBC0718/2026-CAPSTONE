// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/SpecialRoomTransitionSubsystem.h"

void USpecialRoomTransitionSubsystem::SaveReturnState(const FSpecialRoomReturnState& ReturnState)
{
	PendingReturnState = ReturnState;
	bHasPendingReturn = true;
	bBlockStageAutoStartOnce = true;
	bStageAutoStartSkippedForThisLoad = false;
}

bool USpecialRoomTransitionSubsystem::ConsumeReturnState(FSpecialRoomReturnState& OutReturnState)
{
	if (!bHasPendingReturn)
	{
		return false;
	}

	OutReturnState = PendingReturnState;
	bHasPendingReturn = false;
	bBlockStageAutoStartOnce = !bStageAutoStartSkippedForThisLoad;
	return true;
}

bool USpecialRoomTransitionSubsystem::ShouldSkipStageAutoStart() const
{
	return bHasPendingReturn || bBlockStageAutoStartOnce;
}

void USpecialRoomTransitionSubsystem::MarkStageAutoStartSkipped()
{
	bBlockStageAutoStartOnce = false;
	bStageAutoStartSkippedForThisLoad = true;
}

bool USpecialRoomTransitionSubsystem::TryGetPendingSpecialRoomGridPos(FIntPoint& OutGridPos) const
{
	if (!bHasPendingReturn)
	{
		return false;
	}

	OutGridPos = PendingReturnState.SpecialRoomGridPos;
	return true;
}

bool USpecialRoomTransitionSubsystem::IsSpecialRoomRewardConsumed(const FIntPoint& GridPos) const
{
	return ConsumedSpecialRoomRewardGridPositions.Contains(GridPos);
}

void USpecialRoomTransitionSubsystem::MarkSpecialRoomRewardConsumed(const FIntPoint& GridPos)
{
	ConsumedSpecialRoomRewardGridPositions.Add(GridPos);
}

UCAP_WeaponDataAsset* USpecialRoomTransitionSubsystem::GetSpecialRoomRewardWeapon(const FIntPoint& GridPos) const
{
	if (const TObjectPtr<UCAP_WeaponDataAsset>* WeaponData = SpecialRoomRewardWeaponsByGridPos.Find(GridPos))
	{
		return WeaponData->Get();
	}

	return nullptr;
}

void USpecialRoomTransitionSubsystem::SetSpecialRoomRewardWeapon(const FIntPoint& GridPos, UCAP_WeaponDataAsset* WeaponData)
{
	if (WeaponData)
	{
		SpecialRoomRewardWeaponsByGridPos.FindOrAdd(GridPos) = WeaponData;
	}
}

bool USpecialRoomTransitionSubsystem::IsSpecialRoomShopSlotPurchased(FName SlotKey) const
{
	return PurchasedSpecialRoomShopSlotKeys.Contains(SlotKey);
}

void USpecialRoomTransitionSubsystem::MarkSpecialRoomShopSlotPurchased(FName SlotKey)
{
	if (!SlotKey.IsNone())
	{
		PurchasedSpecialRoomShopSlotKeys.Add(SlotKey);
	}
}

bool USpecialRoomTransitionSubsystem::TryGetSpecialRoomShopSlotOfferId(FName SlotKey, FName& OutOfferId) const
{
	if (const FName* OfferId = SpecialRoomShopOfferIdsBySlotKey.Find(SlotKey))
	{
		OutOfferId = *OfferId;
		return true;
	}

	return false;
}

void USpecialRoomTransitionSubsystem::SetSpecialRoomShopSlotOfferId(FName SlotKey, FName OfferId)
{
	if (!SlotKey.IsNone() && !OfferId.IsNone())
	{
		SpecialRoomShopOfferIdsBySlotKey.FindOrAdd(SlotKey) = OfferId;
	}
}

void USpecialRoomTransitionSubsystem::GetSpecialRoomShopOfferIdsByKeyPrefix(const FString& SlotKeyPrefix, TSet<FName>& OutOfferIds) const
{
	for (const TPair<FName, FName>& Pair : SpecialRoomShopOfferIdsBySlotKey)
	{
		if (Pair.Key.ToString().StartsWith(SlotKeyPrefix) && !Pair.Value.IsNone())
		{
			OutOfferIds.Add(Pair.Value);
		}
	}
}
