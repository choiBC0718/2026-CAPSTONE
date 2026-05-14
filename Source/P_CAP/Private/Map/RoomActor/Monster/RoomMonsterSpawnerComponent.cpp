// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Monster/RoomMonsterSpawnerComponent.h"

#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Map/RoomActor/Monster/RoomMonsterSpawnDataAsset.h"

namespace
{
	struct FRoomMonsterCellPlacement
	{
		FRoomMonsterSpawnPick SpawnPick;
		int32 CellIndex = INDEX_NONE;
		int32 MonsterIndexInCell = 0;
		int32 MonsterCountInCell = 1;
	};
}

URoomMonsterSpawnerComponent::URoomMonsterSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URoomMonsterSpawnerComponent::SetSpawnDataAsset(URoomMonsterSpawnDataAsset* InSpawnDataAsset)
{
	if (InSpawnDataAsset)
	{
		SpawnDataAsset = InSpawnDataAsset;
	}
}

void URoomMonsterSpawnerComponent::SpawnMonsters(
	const FRoomData& RoomData,
	const FRoomInteriorLayout& InteriorLayout,
	int32 MapSeed,
	const FTransform& RoomTransform)
{
	ClearSpawnedMonsters();

	if (!SpawnDataAsset)
	{
		return;
	}

	const FRoomMonsterSpawnRule* SpawnRule = SpawnDataAsset->FindRule(RoomData.RoomType);
	if (!SpawnRule)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FRandomStream RandomStream = MakeRoomRandomStream(RoomData, MapSeed);
	TArray<FRoomMonsterSpawnPick> MonsterSpawnList = BuildMonsterSpawnList(*SpawnRule, RandomStream);
	if (MonsterSpawnList.IsEmpty())
	{
		return;
	}

	TArray<FIntPoint> SpawnableCells = CollectSpawnableCells(InteriorLayout);
	if (SpawnableCells.IsEmpty())
	{
		return;
	}

	SortCellsByCenterBias(SpawnableCells, InteriorLayout, RandomStream);

	TArray<TMap<TSubclassOf<ACharacter>, int32>> CellMonsterCounts;
	CellMonsterCounts.SetNum(SpawnableCells.Num());

	TArray<TSubclassOf<ACharacter>> CellMonsterClasses;
	CellMonsterClasses.SetNum(SpawnableCells.Num());

	TArray<FRoomMonsterCellPlacement> Placements;
	Placements.Reserve(MonsterSpawnList.Num());

	for (const FRoomMonsterSpawnPick& SpawnPick : MonsterSpawnList)
	{
		for (int32 CellIndex = 0; CellIndex < SpawnableCells.Num(); ++CellIndex)
		{
			if (CellMonsterClasses[CellIndex] && CellMonsterClasses[CellIndex] != SpawnPick.MonsterClass)
			{
				continue;
			}

			const int32 CurrentCellCount = CellMonsterCounts[CellIndex].FindRef(SpawnPick.MonsterClass);
			if (CurrentCellCount >= FMath::Max(1, SpawnPick.MaxMonstersPerCell))
			{
				continue;
			}

			FRoomMonsterCellPlacement& Placement = Placements.AddDefaulted_GetRef();
			Placement.SpawnPick = SpawnPick;
			Placement.CellIndex = CellIndex;
			Placement.MonsterIndexInCell = CurrentCellCount;

			CellMonsterCounts[CellIndex].FindOrAdd(SpawnPick.MonsterClass)++;
			CellMonsterClasses[CellIndex] = SpawnPick.MonsterClass;
			break;
		}
	}

	TArray<int32> TotalMonsterCountsByCell;
	TotalMonsterCountsByCell.Init(0, SpawnableCells.Num());
	for (const FRoomMonsterCellPlacement& Placement : Placements)
	{
		if (TotalMonsterCountsByCell.IsValidIndex(Placement.CellIndex))
		{
			TotalMonsterCountsByCell[Placement.CellIndex]++;
		}
	}

	TArray<int32> SpawnedMonsterCountsByCell;
	SpawnedMonsterCountsByCell.Init(0, SpawnableCells.Num());

	for (FRoomMonsterCellPlacement& Placement : Placements)
	{
		if (!SpawnableCells.IsValidIndex(Placement.CellIndex))
		{
			continue;
		}

		Placement.MonsterIndexInCell = SpawnedMonsterCountsByCell[Placement.CellIndex]++;
		Placement.MonsterCountInCell = TotalMonsterCountsByCell[Placement.CellIndex];

		FVector LocalLocation = GetCellLocalCenter(InteriorLayout, SpawnableCells[Placement.CellIndex]);
		LocalLocation += GetFormationOffset(Placement.MonsterIndexInCell, Placement.MonsterCountInCell, InteriorLayout.CellSize);
		LocalLocation.X += RandomStream.FRandRange(-SpawnJitter, SpawnJitter);
		LocalLocation.Y += RandomStream.FRandRange(-SpawnJitter, SpawnJitter);
		LocalLocation.Z += SpawnZOffset;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ACharacter* SpawnedMonster = World->SpawnActor<ACharacter>(
			Placement.SpawnPick.MonsterClass,
			RoomTransform.TransformPosition(LocalLocation),
			FRotator(0.f, RandomStream.FRandRange(0.f, 360.f), 0.f),
			SpawnParams);

		if (SpawnedMonster)
		{
			SpawnedMonster->SpawnDefaultController();
			SpawnedMonsters.Add(SpawnedMonster);
		}
	}
}

void URoomMonsterSpawnerComponent::ClearSpawnedMonsters()
{
	for (ACharacter* Monster : SpawnedMonsters)
	{
		if (IsValid(Monster))
		{
			Monster->Destroy();
		}
	}

	SpawnedMonsters.Empty();
}

TArray<FRoomMonsterSpawnPick> URoomMonsterSpawnerComponent::BuildMonsterSpawnList(
	const FRoomMonsterSpawnRule& SpawnRule,
	FRandomStream& RandomStream) const
{
	TArray<FRoomMonsterSpawnPick> MonsterSpawnList;

	const int32 MinScore = FMath::Max(0, FMath::Min(SpawnRule.ScoreRange.X, SpawnRule.ScoreRange.Y));
	const int32 MaxScore = FMath::Max(0, FMath::Max(SpawnRule.ScoreRange.X, SpawnRule.ScoreRange.Y));
	if (MaxScore <= 0 || SpawnRule.MonsterPool.IsEmpty())
	{
		return MonsterSpawnList;
	}

	int32 RemainingScore = RandomStream.RandRange(MinScore, MaxScore);
	TMap<TSubclassOf<ACharacter>, int32> SelectedCounts;

	while (RemainingScore > 0)
	{
		const FRoomMonsterSpawnEntry* PickedEntry = PickMonsterEntry(
			SpawnRule,
			SelectedCounts,
			RemainingScore,
			RandomStream);

		if (!PickedEntry || !PickedEntry->MonsterClass)
		{
			break;
		}

		FRoomMonsterSpawnPick& SpawnPick = MonsterSpawnList.AddDefaulted_GetRef();
		SpawnPick.MonsterClass = PickedEntry->MonsterClass;
		SpawnPick.MaxMonstersPerCell = PickedEntry->MaxMonstersPerCell;

		SelectedCounts.FindOrAdd(PickedEntry->MonsterClass)++;
		RemainingScore -= FMath::Max(1, PickedEntry->Cost);
	}

	return MonsterSpawnList;
}

const FRoomMonsterSpawnEntry* URoomMonsterSpawnerComponent::PickMonsterEntry(
	const FRoomMonsterSpawnRule& SpawnRule,
	const TMap<TSubclassOf<ACharacter>, int32>& SelectedCounts,
	int32 RemainingScore,
	FRandomStream& RandomStream) const
{
	int32 TotalWeight = 0;

	for (const FRoomMonsterSpawnEntry& Entry : SpawnRule.MonsterPool)
	{
		if (!Entry.MonsterClass || Entry.Cost > RemainingScore || Entry.Weight <= 0)
		{
			continue;
		}

		const int32 CurrentCount = SelectedCounts.FindRef(Entry.MonsterClass);
		if (CurrentCount >= Entry.MaxCount)
		{
			continue;
		}

		TotalWeight += Entry.Weight;
	}

	if (TotalWeight <= 0)
	{
		return nullptr;
	}

	const int32 RandomPick = RandomStream.RandRange(1, TotalWeight);
	int32 RunningWeight = 0;

	for (const FRoomMonsterSpawnEntry& Entry : SpawnRule.MonsterPool)
	{
		if (!Entry.MonsterClass || Entry.Cost > RemainingScore || Entry.Weight <= 0)
		{
			continue;
		}

		const int32 CurrentCount = SelectedCounts.FindRef(Entry.MonsterClass);
		if (CurrentCount >= Entry.MaxCount)
		{
			continue;
		}

		RunningWeight += Entry.Weight;
		if (RandomPick <= RunningWeight)
		{
			return &Entry;
		}
	}

	return nullptr;
}

TArray<FIntPoint> URoomMonsterSpawnerComponent::CollectSpawnableCells(
	const FRoomInteriorLayout& InteriorLayout) const
{
	TArray<FIntPoint> SpawnableCells;
	SpawnableCells.Reserve(InteriorLayout.Cells.Num());

	for (const FRoomInteriorCell& Cell : InteriorLayout.Cells)
	{
		if (Cell.Type == ERoomInteriorCellType::Empty &&
			!IsNearReservedDoorCell(InteriorLayout, Cell.Coord) &&
			!IsNearRoomEdge(InteriorLayout, Cell.Coord))
		{
			SpawnableCells.Add(Cell.Coord);
		}
	}

	return SpawnableCells;
}

bool URoomMonsterSpawnerComponent::IsNearReservedDoorCell(
	const FRoomInteriorLayout& InteriorLayout,
	const FIntPoint& CellCoord) const
{
	const int32 SafeRadius = FMath::Max(0, DoorExclusionRadiusInCells);
	if (SafeRadius <= 0)
	{
		return false;
	}

	for (const FRoomInteriorCell& Cell : InteriorLayout.Cells)
	{
		if (Cell.Type != ERoomInteriorCellType::ReservedDoor)
		{
			continue;
		}

		const int32 DistanceX = FMath::Abs(Cell.Coord.X - CellCoord.X);
		const int32 DistanceY = FMath::Abs(Cell.Coord.Y - CellCoord.Y);
		if (FMath::Max(DistanceX, DistanceY) <= SafeRadius)
		{
			return true;
		}
	}

	return false;
}

bool URoomMonsterSpawnerComponent::IsNearRoomEdge(
	const FRoomInteriorLayout& InteriorLayout,
	const FIntPoint& CellCoord) const
{
	const int32 SafeRadius = FMath::Max(0, WallExclusionRadiusInCells);
	if (SafeRadius <= 0)
	{
		return false;
	}

	return CellCoord.X < SafeRadius ||
		CellCoord.Y < SafeRadius ||
		CellCoord.X >= InteriorLayout.GridWidth - SafeRadius ||
		CellCoord.Y >= InteriorLayout.GridHeight - SafeRadius;
}

void URoomMonsterSpawnerComponent::SortCellsByCenterBias(
	TArray<FIntPoint>& SpawnableCells,
	const FRoomInteriorLayout& InteriorLayout,
	FRandomStream& RandomStream) const
{
	if (SpawnableCells.Num() <= 1)
	{
		return;
	}

	const FVector2D GridCenter(
		(InteriorLayout.GridWidth - 1) * 0.5f,
		(InteriorLayout.GridHeight - 1) * 0.5f);
	const float MaxDistance = FVector2D(GridCenter).Size();
	const float SafeMaxDistance = FMath::Max(KINDA_SMALL_NUMBER, MaxDistance);
	const float BiasStrength = FMath::Max(0.f, CenterSpawnBias);

	TMap<FIntPoint, float> CellScores;
	CellScores.Reserve(SpawnableCells.Num());
	for (const FIntPoint& CellCoord : SpawnableCells)
	{
		const FVector2D CellPoint(CellCoord.X, CellCoord.Y);
		const float NormalizedDistance = (CellPoint - GridCenter).Size() / SafeMaxDistance;
		CellScores.Add(CellCoord, NormalizedDistance - RandomStream.FRandRange(0.f, BiasStrength));
	}

	SpawnableCells.Sort(
		[&](const FIntPoint& A, const FIntPoint& B)
		{
			return CellScores.FindRef(A) < CellScores.FindRef(B);
		});
}

FVector URoomMonsterSpawnerComponent::GetCellLocalCenter(
	const FRoomInteriorLayout& InteriorLayout,
	const FIntPoint& CellCoord) const
{
	const float GridWorldSizeX = InteriorLayout.GridWidth * InteriorLayout.CellSize;
	const float GridWorldSizeY = InteriorLayout.GridHeight * InteriorLayout.CellSize;
	const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, 0.f);

	return FVector(
		GridMin.X + (CellCoord.X * InteriorLayout.CellSize) + (InteriorLayout.CellSize * 0.5f),
		GridMin.Y + (CellCoord.Y * InteriorLayout.CellSize) + (InteriorLayout.CellSize * 0.5f),
		0.f);
}

FVector URoomMonsterSpawnerComponent::GetFormationOffset(
	int32 MonsterIndexInCell,
	int32 MonsterCountInCell,
	float CellSize) const
{
	if (MonsterCountInCell <= 1)
	{
		return FVector::ZeroVector;
	}

	const float Radius = FMath::Clamp(CellSize * 0.22f, 35.f, 90.f);
	const float AngleStep = 360.f / MonsterCountInCell;
	const float AngleDegrees = AngleStep * MonsterIndexInCell;

	return FVector(
		FMath::Cos(FMath::DegreesToRadians(AngleDegrees)) * Radius,
		FMath::Sin(FMath::DegreesToRadians(AngleDegrees)) * Radius,
		0.f);
}

FRandomStream URoomMonsterSpawnerComponent::MakeRoomRandomStream(
	const FRoomData& RoomData,
	int32 MapSeed) const
{
	int32 Seed = MapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(RoomData.GridPos));
	Seed = HashCombineFast(Seed, 0x5C2F913B);
	return FRandomStream(Seed);
}
