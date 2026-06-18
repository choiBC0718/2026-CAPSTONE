// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomDecorationSet.h"

const FRoomSmallDecorationEntry* URoomDecorationSet::PickSmallEntry(FRandomStream& RandomStream) const
{
	float TotalWeight = 0.f;
	for (const FRoomSmallDecorationEntry& Entry : SmallDecorEntries)
	{
		if (Entry.Mesh && Entry.Weight > 0)
		{
			TotalWeight += Entry.Weight;
		}
	}

	if (TotalWeight <= 0)
	{
		return nullptr;
	}

	const float RandomPick = RandomStream.FRandRange(0.f, TotalWeight);
	float RunningWeight = 0.f;
	for (const FRoomSmallDecorationEntry& Entry : SmallDecorEntries)
	{
		if (!Entry.Mesh || Entry.Weight <= 0)
		{
			continue;
		}

		RunningWeight += Entry.Weight;
		if (RandomPick <= RunningWeight)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FRoomLargeDecorationEntry* URoomDecorationSet::PickLargeEntry(FRandomStream& RandomStream) const
{
	float TotalWeight = 0.f;
	for (const FRoomLargeDecorationEntry& Entry : LargeDecorEntries)
	{
		if (Entry.ActorClass && Entry.Weight > 0)
		{
			TotalWeight += Entry.Weight;
		}
	}

	if (TotalWeight <= 0)
	{
		return nullptr;
	}

	const float RandomPick = RandomStream.FRandRange(0.f, TotalWeight);
	float RunningWeight = 0.f;
	for (const FRoomLargeDecorationEntry& Entry : LargeDecorEntries)
	{
		if (!Entry.ActorClass || Entry.Weight <= 0)
		{
			continue;
		}

		RunningWeight += Entry.Weight;
		if (RandomPick <= RunningWeight)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FRoomTemplateDecorationRule* URoomDecorationSet::PickTemplateRule(const FRoomData& RoomData, FRandomStream& RandomStream) const
{
	(void)RoomData;

	float TotalWeight = 0.f;
	for (const FRoomTemplateDecorationRule& Rule : TemplateRules)
	{
		if (Rule.TemplateClass && Rule.Weight > 0)
		{
			TotalWeight += Rule.Weight;
		}
	}

	if (TotalWeight <= 0)
	{
		return nullptr;
	}

	const float RandomPick = RandomStream.FRandRange(0.f, TotalWeight);
	float RunningWeight = 0.f;
	for (const FRoomTemplateDecorationRule& Rule : TemplateRules)
	{
		if (!Rule.TemplateClass || Rule.Weight <= 0)
		{
			continue;
		}

		RunningWeight += Rule.Weight;
		if (RandomPick <= RunningWeight)
		{
			return &Rule;
		}
	}

	return nullptr;
}
