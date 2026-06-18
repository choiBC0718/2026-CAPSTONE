// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"

#include "Map/RoomActor/Interior/RoomInteriorGrid.h"
#include "Map/RoomActor/Interior/RoomInteriorPathBuilder.h"

FRoomInteriorLayout URoomInteriorGenerator::GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const
{
	FRoomInteriorLayout Layout;
	(void)Margin;
	(void)MapSeed;

	FRoomInteriorPathBuilder::BuildGuaranteedPaths(Layout, RoomData, RoomHalfExtent);
	FRoomInteriorGrid::Initialize(Layout, RoomHalfExtent, CellSize);
	FRoomInteriorGrid::MarkDoorReservedCells(Layout, RoomData, RoomHalfExtent);
	FRoomInteriorGrid::MarkGuaranteedPathCells(Layout, RoomHalfExtent);

	return Layout;
}
