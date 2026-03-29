// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DoorDirection.generated.h"

/**
 */

UENUM(BlueprintType)
enum class EDoorDirection : uint8
{
	Up    UMETA(DisplayName="Up"),
	Down  UMETA(DisplayName="Down"),
	Left  UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right")
};