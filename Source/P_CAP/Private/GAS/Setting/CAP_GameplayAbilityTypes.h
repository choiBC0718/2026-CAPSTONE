// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_GameplayAbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputType : uint8
{
	None			UMETA(DisplayName = "None"),
	BasicAttack		UMETA(DisplayName = "BasicAttack"),
	Skill1			UMETA(DisplayName = "Skill1"),
	Skill2			UMETA(DisplayName = "Skill2"),
	Dodge			UMETA(DisplayName = "Dodge"),
};

USTRUCT(BlueprintType)
struct FWeaponBaseStats : public FTableRowBase
{
	GENERATED_BODY()
};