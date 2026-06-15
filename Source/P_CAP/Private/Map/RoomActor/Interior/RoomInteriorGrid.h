// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Map/RoomData.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"

class FRoomInteriorGrid
{
public:
	static void Initialize(FRoomInteriorLayout& OutLayout, float RoomHalfExtent, float CellSize);
	static void MarkDoorReservedCells(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent);
	static void MarkGuaranteedPathCells(FRoomInteriorLayout& OutLayout, float RoomHalfExtent);

	static FIntPoint LocalPointToCell(const FRoomInteriorLayout& Layout, const FVector& LocalPoint, float RoomHalfExtent);
	static bool IsValidCell(const FRoomInteriorLayout& Layout, const FIntPoint& Coord);
	static int32 GetCellIndex(const FRoomInteriorLayout& Layout, const FIntPoint& Coord);
	static ERoomInteriorCellType GetCellType(const FRoomInteriorLayout& Layout, const FIntPoint& Coord);
	static void SetCellType(FRoomInteriorLayout& OutLayout, const FIntPoint& Coord, ERoomInteriorCellType NewType);
};
