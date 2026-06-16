// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"

void UCAP_ProgressionSubsystem::SavePlayerProgression(const FPlayerProgressionData& InData)
{
	const float ExistingBonusMaxHealth = CurrentRunData.BonusMaxHealth;
	CurrentRunData = InData;
	CurrentRunData.BonusMaxHealth = ExistingBonusMaxHealth;
}

bool UCAP_ProgressionSubsystem::LoadPlayerProgression(FPlayerProgressionData& OutData)
{
	if (!CurrentRunData.bIsValid)
		return false;

	OutData=CurrentRunData;
	return true;
}

void UCAP_ProgressionSubsystem::AddBonusMaxHealth(float Amount)
{
	if (Amount <= 0.f)
	{
		return;
	}

	CurrentRunData.bIsValid = true;
	CurrentRunData.BonusMaxHealth += Amount;
}

void UCAP_ProgressionSubsystem::ClearProgression()
{
	CurrentRunData = FPlayerProgressionData();
}

void UCAP_ProgressionSubsystem::StartRunTimer(float CurrentTime)
{
	CurrentRunStats.RunStartTime = CurrentTime;
}

void UCAP_ProgressionSubsystem::EndRunTimer(float CurrentTime)
{
	CurrentRunStats.TotalPlayTime += (CurrentTime - CurrentRunStats.RunStartTime);
}

void UCAP_ProgressionSubsystem::AddDamageDeal(float Damage)
{
	if (Damage<=0)
		return;
	CurrentRunStats.TotalDamageDeal += Damage;
	if (Damage>CurrentRunStats.MaxDamageDeal)
		CurrentRunStats.MaxDamageDeal = Damage;
}

void UCAP_ProgressionSubsystem::AddDamageTaken(float Damage)
{
	if (Damage>0)
		CurrentRunStats.TotalDamageTaken += Damage;
}

void UCAP_ProgressionSubsystem::AddHealing(float Healing)
{
	if (Healing>0)
		CurrentRunStats.TotalHealing += Healing;
}

void UCAP_ProgressionSubsystem::AddEnemyDefeated()
{
	CurrentRunStats.EnemiesDefeated++;
}

void UCAP_ProgressionSubsystem::AddCurrencyCnt(ECurrencyType Type, int32 Amount)
{
	if (Amount<=0)
		return;
	switch (Type)
	{
	case ECurrencyType::Gold:
		CurrentRunStats.TotalGetGold += Amount;
		break;
	case ECurrencyType::WeaponMaterial:
		CurrentRunStats.TotalGetWeaponMaterial += Amount;
		break;
	case ECurrencyType::MagicStone:
		CurrentRunStats.TotalGetMagicStone += Amount;
		break;
	}
}

void UCAP_ProgressionSubsystem::ClearRunStats()
{
	CurrentRunData = FPlayerProgressionData();
	CurrentRunStats = FRunStatistics();
}
