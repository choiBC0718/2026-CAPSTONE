// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomData.h"
#include "AI/PlayerBehaviorLearner.h"
#include "RoomMonsterSpawnerComponent.generated.h"

class ACharacter;
class URoomMonsterSpawnDataAsset;
struct FRoomMonsterSpawnRule;
struct FRoomMonsterSpawnEntry;

struct FRoomMonsterSpawnPick
{
	TSubclassOf<ACharacter> MonsterClass;
	int32 MaxMonstersPerCell = 1;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class URoomMonsterSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URoomMonsterSpawnerComponent();

	void SetSpawnDataAsset(URoomMonsterSpawnDataAsset* InSpawnDataAsset);

	void SpawnMonsters(
		const FRoomData& RoomData,
		const FRoomInteriorLayout& InteriorLayout,
		int32 MapSeed,
		const FTransform& RoomTransform,
		const FPlayerTendencyModifier& Tendency = FPlayerTendencyModifier{},
		ERoomZone Zone = ERoomZone::Mid);

	void ClearSpawnedMonsters();

	void ActivateSpawnedMonsters(AActor* TargetActor);
	void DeactivateSpawnedMonsters();
	bool HasSpawnedMonsters() const;
	bool AreAllSpawnedMonstersDefeated() const;
	int32 GetNumSpawnedMonsters() const { return SpawnedMonsters.Num(); }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true"))
	TObjectPtr<URoomMonsterSpawnDataAsset> SpawnDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float SpawnZOffset = 95.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float SpawnJitter = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true", ClampMin="0"))
	int32 DoorExclusionRadiusInCells = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true", ClampMin="0"))
	int32 WallExclusionRadiusInCells = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float CenterSpawnBias = 1.5f;

	UPROPERTY()
	TArray<TObjectPtr<ACharacter>> SpawnedMonsters;

	TArray<FRoomMonsterSpawnPick> BuildMonsterSpawnList(
		const FRoomMonsterSpawnRule& SpawnRule,
		FRandomStream& RandomStream) const;

	const FRoomMonsterSpawnEntry* PickMonsterEntry(
		const FRoomMonsterSpawnRule& SpawnRule,
		const TMap<TSubclassOf<ACharacter>, int32>& SelectedCounts,
		int32 RemainingScore,
		FRandomStream& RandomStream) const;

	TArray<FIntPoint> CollectSpawnableCells(const FRoomInteriorLayout& InteriorLayout) const;
	bool IsNearReservedDoorCell(const FRoomInteriorLayout& InteriorLayout, const FIntPoint& CellCoord) const;
	bool IsNearRoomEdge(const FRoomInteriorLayout& InteriorLayout, const FIntPoint& CellCoord) const;
	void SortCellsByCenterBias(TArray<FIntPoint>& SpawnableCells, const FRoomInteriorLayout& InteriorLayout, FRandomStream& RandomStream) const;
	FVector GetCellLocalCenter(const FRoomInteriorLayout& InteriorLayout, const FIntPoint& CellCoord) const;
	FVector GetFormationOffset(int32 MonsterIndexInCell, int32 MonsterCountInCell, float CellSize) const;
	FRandomStream MakeRoomRandomStream(const FRoomData& RoomData, int32 MapSeed) const;
};
