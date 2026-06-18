// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorPathBuilder.h"

void FRoomInteriorPathBuilder::BuildGuaranteedPaths(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent)
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
		FRoomInteriorPath EntryPath;
		EntryPath.CorridorWidth = CorridorWidth;
		EntryPath.PathPoints.Add(DoorAnchors[0]);
		EntryPath.PathPoints.Add(HubPoint);
		OutLayout.GuaranteedPaths.Add(EntryPath);
		OutLayout.GuaranteedPaths.Add(BuildDeadEndLoopPath(RoomHalfExtent, CorridorWidth));
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

TArray<FVector> FRoomInteriorPathBuilder::GetDoorAnchors(const FRoomData& RoomData, float RoomHalfExtent)
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

FRoomInteriorPath FRoomInteriorPathBuilder::BuildDeadEndLoopPath(float RoomHalfExtent, float CorridorWidth)
{
	FRoomInteriorPath LoopPath;
	LoopPath.CorridorWidth = CorridorWidth;
	LoopPath.bClosedLoop = true;

	const float LoopRadius = FMath::Max(RoomHalfExtent * 0.2f, 450.f);
	const int32 SegmentCount = 16;
	LoopPath.PathPoints.Reserve(SegmentCount);

	for (int32 Index = 0; Index < SegmentCount; ++Index)
	{
		const float AngleDeg = (360.f / SegmentCount) * Index;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);

		LoopPath.PathPoints.Add(FVector(
			FMath::Cos(AngleRad) * LoopRadius,
			FMath::Sin(AngleRad) * LoopRadius,
			0.f));
	}

	return LoopPath;
}

FVector FRoomInteriorPathBuilder::GetHubPoint()
{
	return FVector::ZeroVector;
}
