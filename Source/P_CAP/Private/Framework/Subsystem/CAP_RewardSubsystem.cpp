// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Subsystem/CAP_RewardSubsystem.h"

#include "Framework/CAP_RewardSettings.h"

void UCAP_RewardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	EpicFailCount = 0;
	LegendaryFailCount = 0;
}

EChestGrade UCAP_RewardSubsystem::GetNextChestGrade(ECombatRoomRewardType RoomType)
{
	const UCAP_RewardSettings* Settings = GetDefault<UCAP_RewardSettings>();
	if (!Settings)
		return EChestGrade::Normal;
	
	FRoomRewardChance BaseChance;
	if (Settings->RoomTypeBaseChances.Contains(RoomType))
	{
		BaseChance = Settings->RoomTypeBaseChances[RoomType];
	}
	
	float LegendaryChance = BaseChance.LegendaryChance + (LegendaryFailCount * Settings->LegendaryBonusChancePerFail);
	
	if (FMath::FRandRange(0.0f, 100.0f) <= LegendaryChance)
	{
		LegendaryFailCount = 0; 
		EpicFailCount = 0;      
		return EChestGrade::Legendary;
	}
	
	float EpicChance = BaseChance.EpicChance + (EpicFailCount * Settings->EpicBonusChancePerFail);
	
	if (FMath::FRandRange(0.0f, 100.0f) <= EpicChance)
	{
		LegendaryFailCount++; 
		EpicFailCount = 0;    
		return EChestGrade::Epic;
	}
	
	float RareChance = BaseChance.RareChance;
	if (FMath::FRandRange(0.0f, 100.0f) <= RareChance)
	{
		LegendaryFailCount++;
		EpicFailCount++;
		return EChestGrade::Rare;
	}
	
	LegendaryFailCount++;
	EpicFailCount++;
	return EChestGrade::Normal;
}

