// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Map/RoomData.h"
#include "RoomInteriorData.h"
#include "RoomInteriorGenerator.generated.h"

UCLASS()
class URoomInteriorGenerator : public UObject
{
	GENERATED_BODY()

public:
	FRoomInteriorLayout GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const;
};
