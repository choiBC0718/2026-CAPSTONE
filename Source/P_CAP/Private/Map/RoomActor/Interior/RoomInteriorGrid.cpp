// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorGrid.h"

#include "Map/RoomActor/Interior/RoomInteriorPathBuilder.h"

void FRoomInteriorGrid::Initialize(FRoomInteriorLayout& OutLayout, float RoomHalfExtent, float CellSize)
{
	const float SafeCellSize = FMath::Max(CellSize, 1.f);
	const float RoomSize = RoomHalfExtent * 2.f;
	const int32 GridSize = FMath::Max(FMath::FloorToInt(RoomSize / SafeCellSize), 1);

	OutLayout.GridWidth = GridSize;
	OutLayout.GridHeight = GridSize;
	OutLayout.CellSize = SafeCellSize;
	OutLayout.Cells.Empty();
	OutLayout.Cells.Reserve(GridSize * GridSize);

	for (int32 Y = 0; Y < GridSize; ++Y)
	{
		for (int32 X = 0; X < GridSize; ++X)
		{
			FRoomInteriorCell& NewCell = OutLayout.Cells.AddDefaulted_GetRef();
			NewCell.Coord = FIntPoint(X, Y);
			NewCell.Type = ERoomInteriorCellType::Empty;
		}
	}
}

void FRoomInteriorGrid::MarkDoorReservedCells(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent)
{
	const TArray<FVector> DoorAnchors = FRoomInteriorPathBuilder::GetDoorAnchors(RoomData, RoomHalfExtent);

	for (const FVector& DoorAnchor : DoorAnchors)
	{
		const FIntPoint DoorCell = LocalPointToCell(OutLayout, DoorAnchor, RoomHalfExtent);
		SetCellType(OutLayout, DoorCell, ERoomInteriorCellType::ReservedDoor);

		const FVector DirectionToCenter = (-DoorAnchor).GetSafeNormal2D();
		for (int32 Step = 1; Step <= 2; ++Step)
		{
			const FVector OffsetPoint = DoorAnchor + (DirectionToCenter * OutLayout.CellSize * Step);
			SetCellType(OutLayout, LocalPointToCell(OutLayout, OffsetPoint, RoomHalfExtent), ERoomInteriorCellType::ReservedDoor);
		}
	}
}

void FRoomInteriorGrid::MarkGuaranteedPathCells(FRoomInteriorLayout& OutLayout, float RoomHalfExtent)
{
	for (const FRoomInteriorPath& Path : OutLayout.GuaranteedPaths)
	{
		if (Path.PathPoints.Num() < 2)
		{
			continue;
		}

		const float ReserveRadius = FMath::Max(Path.CorridorWidth * 0.5f, OutLayout.CellSize);
		const int32 SampleCountPerSegment = 8;

		for (int32 Index = 0; Index + 1 < Path.PathPoints.Num(); ++Index)
		{
			const FVector Start = Path.PathPoints[Index];
			const FVector End = Path.PathPoints[Index + 1];

			for (int32 SampleIndex = 0; SampleIndex <= SampleCountPerSegment; ++SampleIndex)
			{
				const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(SampleCountPerSegment);
				const FVector SamplePoint = FMath::Lerp(Start, End, Alpha);
				const FIntPoint CenterCell = LocalPointToCell(OutLayout, SamplePoint, RoomHalfExtent);
				const int32 RadiusInCells = FMath::Max(FMath::CeilToInt(ReserveRadius / OutLayout.CellSize), 1);

				for (int32 OffsetY = -RadiusInCells; OffsetY <= RadiusInCells; ++OffsetY)
				{
					for (int32 OffsetX = -RadiusInCells; OffsetX <= RadiusInCells; ++OffsetX)
					{
						const FIntPoint TestCell = CenterCell + FIntPoint(OffsetX, OffsetY);
						if (!IsValidCell(OutLayout, TestCell))
						{
							continue;
						}

						SetCellType(OutLayout, TestCell, ERoomInteriorCellType::ReservedPath);
					}
				}
			}
		}
	}
}

FIntPoint FRoomInteriorGrid::LocalPointToCell(const FRoomInteriorLayout& Layout, const FVector& LocalPoint, float RoomHalfExtent)
{
	const float ShiftedX = (LocalPoint.X + RoomHalfExtent) / Layout.CellSize;
	const float ShiftedY = (LocalPoint.Y + RoomHalfExtent) / Layout.CellSize;

	const int32 CellX = FMath::Clamp(FMath::FloorToInt(ShiftedX), 0, Layout.GridWidth - 1);
	const int32 CellY = FMath::Clamp(FMath::FloorToInt(ShiftedY), 0, Layout.GridHeight - 1);
	return FIntPoint(CellX, CellY);
}

bool FRoomInteriorGrid::IsValidCell(const FRoomInteriorLayout& Layout, const FIntPoint& Coord)
{
	return Coord.X >= 0 && Coord.X < Layout.GridWidth
		&& Coord.Y >= 0 && Coord.Y < Layout.GridHeight;
}

int32 FRoomInteriorGrid::GetCellIndex(const FRoomInteriorLayout& Layout, const FIntPoint& Coord)
{
	return Coord.Y * Layout.GridWidth + Coord.X;
}

ERoomInteriorCellType FRoomInteriorGrid::GetCellType(const FRoomInteriorLayout& Layout, const FIntPoint& Coord)
{
	if (!IsValidCell(Layout, Coord))
	{
		return ERoomInteriorCellType::Blocked;
	}

	return Layout.Cells[GetCellIndex(Layout, Coord)].Type;
}

void FRoomInteriorGrid::SetCellType(FRoomInteriorLayout& OutLayout, const FIntPoint& Coord, ERoomInteriorCellType NewType)
{
	if (!IsValidCell(OutLayout, Coord))
	{
		return;
	}

	FRoomInteriorCell& Cell = OutLayout.Cells[GetCellIndex(OutLayout, Coord)];
	if (Cell.Type == ERoomInteriorCellType::ReservedDoor && NewType != ERoomInteriorCellType::ReservedDoor)
	{
		return;
	}

	Cell.Type = NewType;
}
