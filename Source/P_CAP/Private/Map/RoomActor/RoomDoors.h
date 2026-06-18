// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Map/RoomActor/DoorDirection.h"

class ARoomActor;

class FRoomDoors
{
public:
	static void Clear(ARoomActor& Room);
	static void SpawnConnected(ARoomActor& Room);
	static void Spawn(ARoomActor& Room, EDoorDirection Direction);
	static void SetPortalEnabled(ARoomActor& Room, bool bEnabled);
	static FTransform GetTransform(const ARoomActor& Room, EDoorDirection Direction);
	static FIntPoint GetNeighborGridPos(const ARoomActor& Room, EDoorDirection Direction);
};
