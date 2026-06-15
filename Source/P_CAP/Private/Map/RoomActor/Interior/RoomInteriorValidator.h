// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Map/RoomData.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"

class FRoomInteriorValidator
{
public:
	static bool TryPlaceStructure(FRoomInteriorLayout& OutLayout, const FIntPoint& Origin, const FIntPoint& Footprint);
	static bool TryPlaceStructureWithValidation(
		FRoomInteriorLayout& OutLayout,
		const FRoomData& RoomData,
		float RoomHalfExtent,
		const FIntPoint& Origin,
		const FIntPoint& Footprint,
		ERoomInteriorStructureCategory Category);
	static bool AreAllDoorsReachable(const FRoomInteriorLayout& Layout, const FRoomData& RoomData, float RoomHalfExtent);

private:
	static bool HasRequiredStructureSpacing(
		const FRoomInteriorLayout& Layout,
		const FIntPoint& Origin,
		const FIntPoint& Footprint,
		int32 RequiredGapInCells);
};
