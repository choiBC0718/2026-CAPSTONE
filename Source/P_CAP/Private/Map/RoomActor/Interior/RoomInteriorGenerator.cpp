// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"

FRoomInteriorLayout URoomInteriorGenerator::GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const
{
	FRoomInteriorLayout Layout;

	BuildGuaranteedPaths(Layout, RoomData, RoomHalfExtent);

	return Layout;
}

void URoomInteriorGenerator::BuildGuaranteedPaths(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent) const
{
	const TArray<FVector> DoorAnchors = GetDoorAnchors(RoomData, RoomHalfExtent);
	if (DoorAnchors.IsEmpty())
	{
		return;
	}

	const FVector HubPoint = GetHubPoint();
	const float CorridorWidth = FMath::Max(RoomHalfExtent * 0.2f, 100.f);

	if (DoorAnchors.Num() == 1)
	{
		FRoomInteriorPath Path;
		Path.CorridorWidth = CorridorWidth;
		Path.PathPoints.Add(DoorAnchors[0]);
		Path.PathPoints.Add(HubPoint);
		OutLayout.GuaranteedPaths.Add(Path);
		return;
	}

	if (DoorAnchors.Num() == 2)
	{
		FRoomInteriorPath Path;
		Path.CorridorWidth = CorridorWidth;
		Path.PathPoints.Add(DoorAnchors[0]);
		Path.PathPoints.Add(HubPoint);
		Path.PathPoints.Add(DoorAnchors[1]);
		OutLayout.GuaranteedPaths.Add(Path);
		return;
	}

	for (const FVector& DoorAnchor : DoorAnchors)
	{
		FRoomInteriorPath Path;
		Path.CorridorWidth = CorridorWidth;
		Path.PathPoints.Add(DoorAnchor);
		Path.PathPoints.Add(HubPoint);
		OutLayout.GuaranteedPaths.Add(Path);
	}
}

TArray<FVector> URoomInteriorGenerator::GetDoorAnchors(const FRoomData& RoomData, float RoomHalfExtent) const
{
	TArray<FVector> DoorAnchors;

	if (RoomData.bConnectedUp)
	{
		DoorAnchors.Add(FVector(0.f, RoomHalfExtent, 0.f));
	}

	if (RoomData.bConnectedDown)
	{
		DoorAnchors.Add(FVector(0.f, -RoomHalfExtent, 0.f));
	}

	if (RoomData.bConnectedLeft)
	{
		DoorAnchors.Add(FVector(-RoomHalfExtent, 0.f, 0.f));
	}

	if (RoomData.bConnectedRight)
	{
		DoorAnchors.Add(FVector(RoomHalfExtent, 0.f, 0.f));
	}

	return DoorAnchors;
}

FVector URoomInteriorGenerator::GetHubPoint() const
{
	return FVector::ZeroVector;
}
