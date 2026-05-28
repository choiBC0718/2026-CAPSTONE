// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Map/RoomTypes.h"
#include "CombatRewardChoiceTypes.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FCombatRewardChoiceOption : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward Choice")
	ECombatRoomRewardType RewardType = ECombatRoomRewardType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward Choice")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward Choice")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward Choice")
	TObjectPtr<UTexture2D> Image = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward Choice")
	FText BadgeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward Choice")
	int32 Cost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reward Choice")
	int32 SortOrder = 0;
};
