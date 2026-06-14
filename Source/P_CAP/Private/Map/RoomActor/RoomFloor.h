// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ARoomActor;

class FRoomFloor
{
public:
	static void ClearVisualTiles(ARoomActor& Room);
	static void SpawnVisualTiles(ARoomActor& Room);
	static bool ShouldSkipVisualTileAtLocalBounds(const ARoomActor& Room, const FVector& LocalCenter, const FVector& LocalExtent);
	static void ApplyBaseVisibility(ARoomActor& Room);
};
