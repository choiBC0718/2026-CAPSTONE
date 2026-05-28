// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"

void UCAP_ProgressionSubsystem::SavePlayerProgression(const FPlayerProgressionData& InData)
{
	CurrentRunData = InData;
}

bool UCAP_ProgressionSubsystem::LoadPlayerProgression(FPlayerProgressionData& OutData)
{
	if (!CurrentRunData.bIsValid)
		return false;

	OutData=CurrentRunData;
	return true;
}

void UCAP_ProgressionSubsystem::ClearProgression()
{
	CurrentRunData = FPlayerProgressionData();
}
