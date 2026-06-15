// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorDebug.h"

#include "DrawDebugHelpers.h"

void FRoomInteriorDebug::DrawCells(
	UWorld* World,
	const FTransform& RoomTransform,
	const FQuat& RoomQuat,
	const FRoomInteriorLayout& Layout,
	float ZOffset,
	bool bEnabled)
{
	if (!bEnabled || !World || Layout.CellSize <= 0.f)
	{
		return;
	}

	const float CellHalfSize = Layout.CellSize * 0.5f;
	const float GridWorldSizeX = Layout.GridWidth * Layout.CellSize;
	const float GridWorldSizeY = Layout.GridHeight * Layout.CellSize;
	const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, ZOffset + 60.f);

	for (const FRoomInteriorCell& Cell : Layout.Cells)
	{
		FColor DebugColor;
		switch (Cell.Type)
		{
		case ERoomInteriorCellType::ReservedDoor:
			DebugColor = FColor::Green;
			break;

		case ERoomInteriorCellType::ReservedPath:
			DebugColor = FColor::Blue;
			break;

		default:
			continue;
		}

		const FVector LocalCenter(
			GridMin.X + (Cell.Coord.X * Layout.CellSize) + CellHalfSize,
			GridMin.Y + (Cell.Coord.Y * Layout.CellSize) + CellHalfSize,
			GridMin.Z);
		const FVector WorldCenter = RoomTransform.TransformPosition(LocalCenter);

		DrawDebugBox(
			World,
			WorldCenter,
			FVector(CellHalfSize * 0.45f, CellHalfSize * 0.45f, 35.f),
			RoomQuat,
			DebugColor,
			true,
			-1.f,
			1,
			6.f);

		DrawDebugSolidBox(
			World,
			WorldCenter,
			FVector(CellHalfSize * 0.42f, CellHalfSize * 0.42f, 25.f),
			RoomQuat,
			FColor(DebugColor.R, DebugColor.G, DebugColor.B, 72),
			true,
			-1.f,
			1);
	}
}
