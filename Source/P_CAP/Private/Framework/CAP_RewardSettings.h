// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "Map/RoomTypes.h"
#include "CAP_RewardSettings.generated.h"

USTRUCT(BlueprintType)
struct FRoomRewardChance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chance")
	float LegendaryChance = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chance")
	float EpicChance = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chance")
	float RareChance = 30.0f;
};

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Reward System Settings"))
class UCAP_RewardSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// 아이템, 무기 분해 시 획득할 재화 테이블
	UPROPERTY(EditAnywhere, Config, Category="Reward")
	TSoftObjectPtr<UDataTable> DisassembleRewardDT;

	static FName GetRowNameFromGrade(EItemGrade Grade);

	// 레전더리 등급 장인의 기운 쌓이는 확률
	UPROPERTY(Config, EditAnywhere, Category = "Pity System")
	float LegendaryBonusChancePerFail = 1.5f;
	// 에픽 등급 장인의 기운 쌓이는 확률
	UPROPERTY(Config, EditAnywhere, Category = "Pity System")
	float EpicBonusChancePerFail = 3.0f;

	// 방 타입별 기본 드랍 확률
	UPROPERTY(Config, EditAnywhere, Category = "Room Data")
	TMap<ECombatRoomRewardType, FRoomRewardChance> RoomTypeBaseChances;
};
