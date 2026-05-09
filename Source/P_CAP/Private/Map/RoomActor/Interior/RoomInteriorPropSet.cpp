// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorPropSet.h"

const FRoomInteriorPropRule* URoomInteriorPropSet::FindRule(
	ERoomInteriorStructureCategory Category,
	const FIntPoint& Footprint) const
{
	for (const FRoomInteriorPropRule& Rule : Rules)
	{
		if (Rule.Category == Category && Rule.Footprint == Footprint)
		{
			return &Rule;
		}
	}

	const FIntPoint SwappedFootprint(Footprint.Y, Footprint.X);
	for (const FRoomInteriorPropRule& Rule : Rules)
	{
		if (Rule.Category == Category && Rule.Footprint == SwappedFootprint)
		{
			return &Rule;
		}
	}

	for (const FRoomInteriorPropRule& Rule : Rules)
	{
		if (Rule.Category == ERoomInteriorStructureCategory::Generic && Rule.Footprint == Footprint)
		{
			return &Rule;
		}
	}

	for (const FRoomInteriorPropRule& Rule : Rules)
	{
		if (Rule.Category == ERoomInteriorStructureCategory::Generic && Rule.Footprint == SwappedFootprint)
		{
			return &Rule;
		}
	}

	return nullptr;
}
