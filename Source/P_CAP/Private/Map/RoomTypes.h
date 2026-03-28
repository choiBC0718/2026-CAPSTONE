// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomTypes.generated.h"

/**
 방 종류 구분하는 enum 클래스
 */

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