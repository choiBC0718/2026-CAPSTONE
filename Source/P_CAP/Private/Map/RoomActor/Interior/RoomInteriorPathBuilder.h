// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Map/RoomData.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"

class FRoomInteriorPathBuilder
{
public:
	static void BuildGuaranteedPaths(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent);
	static TArray<FVector> GetDoorAnchors(const FRoomData& RoomData, float RoomHalfExtent);

private:
	static FRoomInteriorPath BuildDeadEndLoopPath(float RoomHalfExtent, float CorridorWidth);
	static FVector GetHubPoint();
};
