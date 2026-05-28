// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomTypes.generated.h"

UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Start   UMETA(DisplayName = "Start"),
	Normal  UMETA(DisplayName = "Normal"),
	Boss    UMETA(DisplayName = "Boss"),
	Reward  UMETA(DisplayName = "Reward"),
	Shop    UMETA(DisplayName = "Shop"),
	Event   UMETA(DisplayName = "Event")
};

UENUM(BlueprintType)
enum class ERoomZone : uint8
{
	Core    UMETA(DisplayName = "Core"),
	Mid     UMETA(DisplayName = "Mid"),
	Outer   UMETA(DisplayName = "Outer")
};

UENUM(BlueprintType)
enum class ECombatRoomRewardType : uint8
{
	None UMETA(DisplayName = "None"),
	Gold UMETA(DisplayName = "Gold"),
	Item UMETA(DisplayName = "Item"),
	Weapon UMETA(DisplayName = "Weapon")
};
