// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"

class FRoomInteriorDebug
{
public:
	static void DrawCells(
		UWorld* World,
		const FTransform& RoomTransform,
		const FQuat& RoomQuat,
		const FRoomInteriorLayout& Layout,
		float ZOffset,
		bool bEnabled);
};
