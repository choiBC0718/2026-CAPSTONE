// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/Reward/CAP_RewardChest.h"
#include "Map/RoomTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CAP_RewardSubsystem.generated.h"


/**
 * 실시간 게임 로직 (연산 매니저), 기획(UDeveloperSettings) 데이터를 통해 계산
 */
UCLASS()
class UCAP_RewardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	EChestGrade GetNextChestGrade(ECombatRoomRewardType RoomType);
private:
	int32 EpicFailCount;
	int32 LegendaryFailCount;
};