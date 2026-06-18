// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ARoomActor;

class FRoomFence
{
public:
	static void Clear(ARoomActor& Room);
	static void Spawn(ARoomActor& Room);

private:
	static void SpawnLine(ARoomActor& Room, const FVector& EdgeStart, const FVector& EdgeDirection, float EdgeLength, float Yaw, bool bHasDoorGap);
	static void SpawnDoorSideBlocks(ARoomActor& Room, const FVector& EdgeStart, const FVector& EdgeDirection, float EdgeLength, float Yaw);
	static bool IsInDoorGap(const ARoomActor& Room, float DistanceAlongEdge, float EdgeLength);
};
