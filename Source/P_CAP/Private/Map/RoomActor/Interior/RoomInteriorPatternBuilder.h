// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Map/RoomData.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"

class FRoomInteriorPatternBuilder
{
public:
	static void PlaceLargeStructurePattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream);
	static void PlaceSecondaryStructures(
		FRoomInteriorLayout& OutLayout,
		const FRoomData& RoomData,
		float RoomHalfExtent,
		FRandomStream& RandomStream);

private:
	static void PlaceDeadEndPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream);
	static void PlaceStraightPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream);
	static void PlaceCornerPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream);
	static void PlaceHubPattern(FRoomInteriorLayout& OutLayout, FRandomStream& RandomStream);
	static void PlaceCandidateStructures(
		FRoomInteriorLayout& OutLayout,
		const TArray<TArray<FRoomInteriorPlacedStructure>>& CandidateGroups,
		FRandomStream& RandomStream);
};
