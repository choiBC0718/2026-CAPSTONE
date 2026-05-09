// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Monster/RoomMonsterSpawnDataAsset.h"

const FRoomMonsterSpawnRule* URoomMonsterSpawnDataAsset::FindRule(ERoomType RoomType) const
{
	for (const FRoomMonsterSpawnRule& Rule : SpawnRules)
	{
		if (Rule.RoomType == RoomType)
		{
			return &Rule;
		}
	}

	return nullptr;
}
