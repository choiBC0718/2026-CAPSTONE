// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Map/RoomTypes.h"
#include "Map/RoomActor/DoorDirection.h"
#include "Map/RoomActor/Monster/RoomMonsterSpawnDataAsset.h"
#include "SpecialRoomTransitionSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FSpecialRoomReturnState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Special Room")
	int32 Seed = 12345;

	UPROPERTY(BlueprintReadOnly, Category="Special Room")
	int32 RoomCount = 10;

	UPROPERTY(BlueprintReadOnly, Category="Special Room")
	FIntPoint ReturnRoomGridPos = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category="Special Room")
	EDoorDirection ReturnEntryDirection = EDoorDirection::Up;

	UPROPERTY(BlueprintReadOnly, Category="Special Room")
	TObjectPtr<URoomMonsterSpawnDataAsset> MonsterSpawnDataAsset;

	TSet<FIntPoint> ClearedRoomGridPositions;
	TMap<FIntPoint, ECombatRoomRewardType> CombatRewardTypesByRoom;
};

UCLASS()
class USpecialRoomTransitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void SaveReturnState(const FSpecialRoomReturnState& ReturnState);
	bool HasPendingReturn() const { return bHasPendingReturn; }
	bool ConsumeReturnState(FSpecialRoomReturnState& OutReturnState);

	bool ShouldSkipStageAutoStart() const;
	void MarkStageAutoStartSkipped();

private:
	UPROPERTY()
	FSpecialRoomReturnState PendingReturnState;

	bool bHasPendingReturn = false;
	bool bBlockStageAutoStartOnce = false;
	bool bStageAutoStartSkippedForThisLoad = false;
};
