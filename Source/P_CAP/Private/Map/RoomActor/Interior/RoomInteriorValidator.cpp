// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorValidator.h"

#include "Containers/Queue.h"
#include "Map/RoomActor/Interior/RoomInteriorGrid.h"
#include "Map/RoomActor/Interior/RoomInteriorPathBuilder.h"

bool FRoomInteriorValidator::TryPlaceStructure(FRoomInteriorLayout& OutLayout, const FIntPoint& Origin, const FIntPoint& Footprint)
{
	constexpr int32 RequiredGapInCells = 1;
	if (!HasRequiredStructureSpacing(OutLayout, Origin, Footprint, RequiredGapInCells))
	{
		return false;
	}

	for (int32 Y = 0; Y < Footprint.Y; ++Y)
	{
		for (int32 X = 0; X < Footprint.X; ++X)
		{
			const FIntPoint Coord = Origin + FIntPoint(X, Y);
			if (!FRoomInteriorGrid::IsValidCell(OutLayout, Coord))
			{
				return false;
			}

			const ERoomInteriorCellType CellType = FRoomInteriorGrid::GetCellType(OutLayout, Coord);
			if (CellType != ERoomInteriorCellType::Empty)
			{
				return false;
			}
		}
	}

	for (int32 Y = 0; Y < Footprint.Y; ++Y)
	{
		for (int32 X = 0; X < Footprint.X; ++X)
		{
			FRoomInteriorGrid::SetCellType(OutLayout, Origin + FIntPoint(X, Y), ERoomInteriorCellType::Blocked);
		}
	}

	FRoomInteriorPlacedStructure& NewStructure = OutLayout.PlacedStructures.AddDefaulted_GetRef();
	NewStructure.Origin = Origin;
	NewStructure.Footprint = Footprint;
	return true;
}

bool FRoomInteriorValidator::TryPlaceStructureWithValidation(
	FRoomInteriorLayout& OutLayout,
	const FRoomData& RoomData,
	float RoomHalfExtent,
	const FIntPoint& Origin,
	const FIntPoint& Footprint,
	ERoomInteriorStructureCategory Category)
{
	FRoomInteriorLayout CandidateLayout = OutLayout;
	if (!TryPlaceStructure(CandidateLayout, Origin, Footprint))
	{
		return false;
	}

	if (!AreAllDoorsReachable(CandidateLayout, RoomData, RoomHalfExtent))
	{
		return false;
	}

	OutLayout = CandidateLayout;
	if (OutLayout.PlacedStructures.Num() > 0)
	{
		OutLayout.PlacedStructures.Last().Category = Category;
	}

	return true;
}

bool FRoomInteriorValidator::AreAllDoorsReachable(const FRoomInteriorLayout& Layout, const FRoomData& RoomData, float RoomHalfExtent)
{
	TArray<FIntPoint> DoorCells;
	const TArray<FVector> DoorAnchors = FRoomInteriorPathBuilder::GetDoorAnchors(RoomData, RoomHalfExtent);
	DoorCells.Reserve(DoorAnchors.Num());

	for (const FVector& DoorAnchor : DoorAnchors)
	{
		const FIntPoint DoorCell = FRoomInteriorGrid::LocalPointToCell(Layout, DoorAnchor, RoomHalfExtent);
		if (FRoomInteriorGrid::IsValidCell(Layout, DoorCell))
		{
			DoorCells.Add(DoorCell);
		}
	}

	if (DoorCells.Num() <= 1)
	{
		return true;
	}

	TSet<FIntPoint> Visited;
	TQueue<FIntPoint> Queue;
	Queue.Enqueue(DoorCells[0]);
	Visited.Add(DoorCells[0]);

	while (!Queue.IsEmpty())
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		static const FIntPoint Directions[] =
		{
			FIntPoint(1, 0),
			FIntPoint(-1, 0),
			FIntPoint(0, 1),
			FIntPoint(0, -1)
		};

		for (const FIntPoint& Direction : Directions)
		{
			const FIntPoint Next = Current + Direction;
			if (!FRoomInteriorGrid::IsValidCell(Layout, Next) || Visited.Contains(Next))
			{
				continue;
			}

			const ERoomInteriorCellType CellType = FRoomInteriorGrid::GetCellType(Layout, Next);
			if (CellType == ERoomInteriorCellType::Blocked)
			{
				continue;
			}

			Visited.Add(Next);
			Queue.Enqueue(Next);
		}
	}

	for (const FIntPoint& DoorCell : DoorCells)
	{
		if (!Visited.Contains(DoorCell))
		{
			return false;
		}
	}

	return true;
}

bool FRoomInteriorValidator::HasRequiredStructureSpacing(
	const FRoomInteriorLayout& Layout,
	const FIntPoint& Origin,
	const FIntPoint& Footprint,
	int32 RequiredGapInCells)
{
	const FIntPoint MinCoord = Origin - FIntPoint(RequiredGapInCells, RequiredGapInCells);
	const FIntPoint MaxCoord = Origin + Footprint - FIntPoint(1, 1) + FIntPoint(RequiredGapInCells, RequiredGapInCells);

	for (int32 Y = MinCoord.Y; Y <= MaxCoord.Y; ++Y)
	{
		for (int32 X = MinCoord.X; X <= MaxCoord.X; ++X)
		{
			const FIntPoint Coord(X, Y);
			if (!FRoomInteriorGrid::IsValidCell(Layout, Coord))
			{
				continue;
			}

			if (FRoomInteriorGrid::GetCellType(Layout, Coord) == ERoomInteriorCellType::Blocked)
			{
				return false;
			}
		}
	}

	return true;
}
