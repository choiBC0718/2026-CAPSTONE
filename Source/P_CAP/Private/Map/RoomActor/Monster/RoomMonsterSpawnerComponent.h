// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomData.h"
#include "AI/PlayerBehaviorLearner.h"
#include "Map/RoomActor/Monster/RoomMonsterSpawnDataAsset.h"
#include "RoomMonsterSpawnerComponent.generated.h"

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
		const FPlayerTendencyModifier& Tendency = FPlayerTendencyModifier{});

	int32 SpawnReinforcement(
		const FRoomData& RoomData,
		const FRoomInteriorLayout& InteriorLayout,
		int32 MapSeed,
		const FTransform& RoomTransform,
		const FPlayerTendencyModifier& Tendency,
		int32 ReinforcementIndex);

	void ClearSpawnedMonsters();

	void ActivateSpawnedMonsters(AActor* TargetActor);
	void DeactivateSpawnedMonsters();
	bool HasSpawnedMonsters() const;
	bool AreAllSpawnedMonstersDefeated() const;
	int32 GetAliveSpawnedMonsterCount() const;
	int32 GetNumSpawnedMonsters() const { return SpawnedMonsters.Num(); }
	const FRoomMonsterSpawnRule* GetSpawnRule(ERoomType RoomType) const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true"))
	TObjectPtr<URoomMonsterSpawnDataAsset> SpawnDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float SpawnZOffset = 95.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float SpawnJitter = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true", ClampMin="0"))
	int32 SpawnLocationMaxRetries = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float SpawnCollisionPadding = 5.f;

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

	int32 SpawnMonstersFromRule(
		const FRoomData& RoomData,
		const FRoomInteriorLayout& InteriorLayout,
		int32 MapSeed,
		const FTransform& RoomTransform,
		const FPlayerTendencyModifier& Tendency,
		const FRoomMonsterSpawnRule& SpawnRule,
		int32 RandomSalt,
		bool bClearExisting,
		const FRoomReinforcementRule* ReinforcementRule = nullptr);

	const FRoomMonsterSpawnEntry* PickMonsterEntry(
		const FRoomMonsterSpawnRule& SpawnRule,
		const TMap<TSubclassOf<ACharacter>, int32>& SelectedCounts,
		int32 RemainingScore,
		FRandomStream& RandomStream) const;

	TArray<FIntPoint> CollectSpawnableCells(const FRoomInteriorLayout& InteriorLayout) const;
	bool IsNearReservedDoorCell(const FRoomInteriorLayout& InteriorLayout, const FIntPoint& CellCoord) const;
	bool IsNearRoomEdge(const FRoomInteriorLayout& InteriorLayout, const FIntPoint& CellCoord) const;
	void SortCellsByCenterBias(TArray<FIntPoint>& SpawnableCells, const FRoomInteriorLayout& InteriorLayout, FRandomStream& RandomStream, float ExplorationRate) const;
	FVector GetCellLocalCenter(const FRoomInteriorLayout& InteriorLayout, const FIntPoint& CellCoord) const;
	FVector GetFormationOffset(int32 MonsterIndexInCell, int32 MonsterCountInCell, float CellSize) const;
	bool TryBuildSpawnLocation(
		TSubclassOf<ACharacter> MonsterClass,
		const FRoomInteriorLayout& InteriorLayout,
		const FIntPoint& CellCoord,
		int32 MonsterIndexInCell,
		int32 MonsterCountInCell,
		const FTransform& RoomTransform,
		FRandomStream& RandomStream,
		FVector& OutWorldLocation) const;
	bool IsSpawnLocationFree(TSubclassOf<ACharacter> MonsterClass, const FVector& WorldLocation) const;
	FRandomStream MakeRoomRandomStream(const FRoomData& RoomData, int32 MapSeed, int32 RandomSalt = 0) const;
};
