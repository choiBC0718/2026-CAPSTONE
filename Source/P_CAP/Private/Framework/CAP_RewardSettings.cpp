// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CAP_RewardSettings.h"

FName UCAP_RewardSettings::GetRowNameFromGrade(EItemGrade Grade)
{
	switch (Grade)
	{
	case EItemGrade::Normal:		return FName("Normal");
	case EItemGrade::Rare:			return FName("Rare");
	case EItemGrade::Epic:			return FName("Epic");
	case EItemGrade::Legendary:		return FName("Legendary");
	default:						return FName();
	}
}
