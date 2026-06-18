// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/RoomObstacle.h"

#include "Map/RoomActor/RoomActor.h"
#include "AI/AnalysisObstacle.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

void FRoomObstacle::Clear(ARoomActor& Room)
{
	for (AAnalysisObstacle* Obstacle : Room.SpawnedObstacles)
	{
		if (IsValid(Obstacle))
		{
			Obstacle->Destroy();
		}
	}
	Room.SpawnedObstacles.Empty();
}

void FRoomObstacle::SpawnByTendency(ARoomActor& Room, const FPlayerTendencyModifier& Tendency)
{
	if (!Room.ObstacleClass || Room.MaxObstaclesPerRoom <= 0)
	{
		return;
	}
	if (Room.CachedRoomData.RoomType != ERoomType::Normal)
	{
		return;
	}

	const float BaseCount = FMath::Lerp(1.f, static_cast<float>(Room.MaxObstaclesPerRoom), Tendency.ObstacleBypass);
	const int32 ObstacleCount = FMath::RoundToInt(BaseCount);

	UE_LOG(LogTemp, Log, TEXT("[Obstacle] Room(%d,%d) | Bypass=%.2f -> obstacles %d"),
		Room.CachedRoomData.GridPos.X, Room.CachedRoomData.GridPos.Y,
		Tendency.ObstacleBypass, ObstacleCount);

	if (ObstacleCount <= 0)
	{
		return;
	}

	UWorld* World = Room.GetWorld();
	if (!World)
	{
		return;
	}

	int32 Seed = Room.CachedMapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(Room.CachedRoomData.GridPos));
	Seed = HashCombineFast(Seed, 0xF1A20B3C);
	FRandomStream RandomStream(Seed);

	TArray<FIntPoint> DoorCoords;
	for (const FRoomInteriorCell& Cell : Room.CachedInteriorLayout.Cells)
	{
		if (Cell.Type == ERoomInteriorCellType::ReservedDoor)
		{
			DoorCoords.Add(Cell.Coord);
		}
	}

	const int32 EdgeMargin = 1;
	TArray<FIntPoint> SpawnableCells;
	for (const FRoomInteriorCell& Cell : Room.CachedInteriorLayout.Cells)
	{
		if (Cell.Type != ERoomInteriorCellType::Empty)
		{
			continue;
		}
		if (Cell.Coord.X < EdgeMargin || Cell.Coord.Y < EdgeMargin ||
			Cell.Coord.X >= Room.CachedInteriorLayout.GridWidth - EdgeMargin ||
			Cell.Coord.Y >= Room.CachedInteriorLayout.GridHeight - EdgeMargin)
		{
			continue;
		}
		SpawnableCells.Add(Cell.Coord);
	}

	float MaxDoorDist = KINDA_SMALL_NUMBER;
	TMap<FIntPoint, float> DoorDistMap;
	for (const FIntPoint& CellCoord : SpawnableCells)
	{
		float MinDist = TNumericLimits<float>::Max();
		for (const FIntPoint& Door : DoorCoords)
		{
			MinDist = FMath::Min(MinDist, FVector2D::Distance(FVector2D(CellCoord), FVector2D(Door)));
		}
		DoorDistMap.Add(CellCoord, MinDist);
		MaxDoorDist = FMath::Max(MaxDoorDist, MinDist);
	}

	TMap<FIntPoint, float> CellScores;
	for (const FIntPoint& CellCoord : SpawnableCells)
	{
		const float NormalizedDist = DoorDistMap.FindRef(CellCoord) / MaxDoorDist;
		const float DirectionScore = FMath::Lerp(NormalizedDist, 1.f - NormalizedDist, Tendency.ExplorationRate);
		CellScores.Add(CellCoord, DirectionScore + RandomStream.FRandRange(-0.05f, 0.05f));
	}

	SpawnableCells.Sort([&](const FIntPoint& A, const FIntPoint& B)
	{
		return CellScores.FindRef(A) < CellScores.FindRef(B);
	});

	const float GridWorldSizeX = Room.CachedInteriorLayout.GridWidth * Room.CachedInteriorLayout.CellSize;
	const float GridWorldSizeY = Room.CachedInteriorLayout.GridHeight * Room.CachedInteriorLayout.CellSize;
	const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, 0.f);
	const float SpawnJitter = Room.CachedInteriorLayout.CellSize * 0.2f;

	int32 CellIndex = 0;
	for (int32 i = 0; i < ObstacleCount; i++)
	{
		FVector LocalPos;
		bool bFound = false;

		while (CellIndex < SpawnableCells.Num())
		{
			const FIntPoint& Coord = SpawnableCells[CellIndex++];
			LocalPos = FVector(
				GridMin.X + Coord.X * Room.CachedInteriorLayout.CellSize + Room.CachedInteriorLayout.CellSize * 0.5f + RandomStream.FRandRange(-SpawnJitter, SpawnJitter),
				GridMin.Y + Coord.Y * Room.CachedInteriorLayout.CellSize + Room.CachedInteriorLayout.CellSize * 0.5f + RandomStream.FRandRange(-SpawnJitter, SpawnJitter),
				0.f);

			bool bTooClose = false;
			for (const TObjectPtr<AAnalysisObstacle>& Existing : Room.SpawnedObstacles)
			{
				if (IsValid(Existing) &&
					FVector::Dist(Room.GetActorTransform().TransformPosition(LocalPos), Existing->GetActorLocation()) < Room.CachedInteriorLayout.CellSize)
				{
					bTooClose = true;
					break;
				}
			}

			if (!bTooClose)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			break;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = &Room;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		const FVector WorldPos = Room.GetActorTransform().TransformPosition(LocalPos);
		const FRotator WorldRot = FRotator(0.f, RandomStream.FRandRange(0.f, 360.f), 0.f);

		AAnalysisObstacle* Spawned = World->SpawnActor<AAnalysisObstacle>(Room.ObstacleClass, WorldPos, WorldRot, SpawnParams);
		if (Spawned)
		{
			Spawned->BypassMonsterClass = Room.BypassMonsterClass;
			Room.SpawnedObstacles.Add(Spawned);

			const float NormalizedScore = CellScores.IsEmpty() ? 0.5f : CellIndex / FMath::Max(1.f, (float)SpawnableCells.Num());
			const FColor DebugColor = NormalizedScore < 0.5f ? FColor::Red : FColor::Blue;
			DrawDebugSphere(World, WorldPos + FVector(0, 0, 150), 80.f, 8, DebugColor, false, 15.f, 0, 3.f);
		}
	}
}
