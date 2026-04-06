// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Math/RandomStream.h"


FRoomInteriorLayout URoomInteriorGenerator::GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const
{
	FRoomInteriorLayout Layout;

	/* Normal 방 내부 랜덤 생성 */
	if (RoomData.RoomType != ERoomType::Normal)
	{
		return Layout;
	}

	BuildCells(Layout, RoomHalfExtent, CellSize, Margin);

	if (Layout.CellsPerSide <= 0 || Layout.Cells.IsEmpty())
	{
		return Layout;
	}

	ReserveDoorCells(Layout, RoomData);

	const int32 RoomSeed = MakeRoomSeed(RoomData, MapSeed);
	PlaceObstacles(Layout, RoomData, RoomSeed);

	return Layout;
}

int32 URoomInteriorGenerator::MakeRoomSeed(const FRoomData& RoomData, int32 MapSeed) const
{
	uint32 SeedHash = ::GetTypeHash(MapSeed);
	SeedHash = HashCombine(SeedHash, ::GetTypeHash(RoomData.GridPos.X));
	SeedHash = HashCombine(SeedHash, ::GetTypeHash(RoomData.GridPos.Y));
	SeedHash = HashCombine(SeedHash, ::GetTypeHash(static_cast<uint8>(RoomData.RoomType)));

	return static_cast<int32>(SeedHash);
}

void URoomInteriorGenerator::BuildCells(FRoomInteriorLayout& OutLayout, float RoomHalfExtent, float CellSize, float Margin) const
{
	const float UsableSize = (RoomHalfExtent - Margin) * 2.f;
	const int32 CellsPerSide = FMath::FloorToInt(UsableSize / CellSize);

	if (CellsPerSide <= 0)
	{
		return;
	}

	OutLayout.CellsPerSide = CellsPerSide;
	OutLayout.Cells.Reserve(CellsPerSide * CellsPerSide);

	const float StartOffset = -((CellsPerSide - 1) * CellSize) * 0.5f;

	for (int32 Y = 0; Y < CellsPerSide; ++Y)
	{
		for (int32 X = 0; X < CellsPerSide; ++X)
		{
			FRoomInteriorCell NewCell;
			NewCell.CellCoord = FIntPoint(X, Y);
			NewCell.LocalPosition = FVector(
				StartOffset + X * CellSize,
				StartOffset + Y * CellSize,
				0.f
			);
			NewCell.CellType = ERoomInteriorCellType::Empty;

			OutLayout.Cells.Add(NewCell);
		}
	}
}

void URoomInteriorGenerator::ReserveDoorCells(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData) const
{
	const int32 N = OutLayout.CellsPerSide;
	if (N <= 0)
	{
		return;
	}

	const int32 Mid = N / 2;
	const int32 HalfDoorWidth = 1; // 중앙 포함 3칸
	const int32 Depth = 2;         // 문 앞 2줄 예약

	if (RoomData.bConnectedUp)
	{
		ReserveCellsRect(OutLayout, Mid - HalfDoorWidth, Mid + HalfDoorWidth, N - Depth, N - 1);
	}

	if (RoomData.bConnectedDown)
	{
		ReserveCellsRect(OutLayout, Mid - HalfDoorWidth, Mid + HalfDoorWidth, 0, Depth - 1);
	}

	if (RoomData.bConnectedLeft)
	{
		ReserveCellsRect(OutLayout, 0, Depth - 1, Mid - HalfDoorWidth, Mid + HalfDoorWidth);
	}

	if (RoomData.bConnectedRight)
	{
		ReserveCellsRect(OutLayout, N - Depth, N - 1, Mid - HalfDoorWidth, Mid + HalfDoorWidth);
	}
}

void URoomInteriorGenerator::PlaceObstacles(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, int32 RoomSeed) const
{
	FRandomStream RandomStream(RoomSeed);

	TArray<int32> CandidateIndices;
	CandidateIndices.Reserve(OutLayout.Cells.Num());

	for (int32 Index = 0; Index < OutLayout.Cells.Num(); ++Index)
	{
		const FRoomInteriorCell& Cell = OutLayout.Cells[Index];

		if (Cell.CellType != ERoomInteriorCellType::Empty)
		{
			continue;
		}

		CandidateIndices.Add(Index);
	}

	if (CandidateIndices.IsEmpty())
	{
		return;
	}

	const int32 ObstacleCount = FMath::Clamp(
		RandomStream.RandRange(2, 5),
		0,
		CandidateIndices.Num()
	);

	for (int32 i = 0; i < ObstacleCount; ++i)
	{
		const int32 PickedCandidateIndex = RandomStream.RandRange(0, CandidateIndices.Num() - 1);
		const int32 CellIndex = CandidateIndices[PickedCandidateIndex];

		if (OutLayout.Cells.IsValidIndex(CellIndex))
		{
			OutLayout.Cells[CellIndex].CellType = ERoomInteriorCellType::Obstacle;
		}

		CandidateIndices.RemoveAtSwap(PickedCandidateIndex);
	}
}

void URoomInteriorGenerator::ReserveCellsRect(FRoomInteriorLayout& OutLayout, int32 MinX, int32 MaxX, int32 MinY, int32 MaxY) const
{
	for (int32 Y = MinY; Y <= MaxY; ++Y)
	{
		for (int32 X = MinX; X <= MaxX; ++X)
		{
			FRoomInteriorCell* Cell = OutLayout.FindCell(X, Y);
			if (!Cell)
			{
				continue;
			}

			Cell->CellType = ERoomInteriorCellType::Reserved;
		}
	}
}
