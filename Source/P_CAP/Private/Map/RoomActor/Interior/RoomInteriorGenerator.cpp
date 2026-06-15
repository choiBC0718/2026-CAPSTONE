// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"

#include "Map/RoomActor/Interior/RoomInteriorGrid.h"
#include "Map/RoomActor/Interior/RoomInteriorPathBuilder.h"
#include "Map/RoomActor/Interior/RoomInteriorPatternBuilder.h"
#include "Map/RoomActor/Interior/RoomInteriorValidator.h"

FRoomInteriorLayout URoomInteriorGenerator::GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const
{
	FRoomInteriorLayout Layout;
	(void)Margin;

	FRoomInteriorPathBuilder::BuildGuaranteedPaths(Layout, RoomData, RoomHalfExtent);
	FRoomInteriorGrid::Initialize(Layout, RoomHalfExtent, CellSize);
	FRoomInteriorGrid::MarkDoorReservedCells(Layout, RoomData, RoomHalfExtent);

	FRandomStream RandomStream = MakeRoomRandomStream(RoomData, MapSeed);
	FRoomInteriorPatternBuilder::PlaceLargeStructurePattern(Layout, RoomData, RandomStream);
	FRoomInteriorPatternBuilder::PlaceSecondaryStructures(Layout, RoomData, RoomHalfExtent, RandomStream);

	if (!FRoomInteriorValidator::AreAllDoorsReachable(Layout, RoomData, RoomHalfExtent))
	{
		for (FRoomInteriorCell& Cell : Layout.Cells)
		{
			if (Cell.Type == ERoomInteriorCellType::Blocked)
			{
				Cell.Type = ERoomInteriorCellType::Empty;
			}
		}

		Layout.PlacedStructures.Empty();
	}

	return Layout;
}

FRandomStream URoomInteriorGenerator::MakeRoomRandomStream(const FRoomData& RoomData, int32 MapSeed) const
{
	int32 Seed = MapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(RoomData.GridPos));
	Seed = HashCombineFast(Seed, static_cast<int32>(RoomData.RoomType));
	Seed = HashCombineFast(Seed, RoomData.GetConnectionCount());
	return FRandomStream(Seed);
}
