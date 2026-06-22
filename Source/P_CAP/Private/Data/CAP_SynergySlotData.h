// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_EquipItemEffectTypes.h"
#include "CAP_SynergySlotData.generated.h"

class UCAP_SynergyDataAsset;
/**
 * 
 */
UCLASS()
class UCAP_SynergySlotData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGameplayTag SynergyTag;

	UPROPERTY()
	UCAP_SynergyDataAsset* SynergyDA;

	UPROPERTY()
	int32 CurrentCount = 0;
};
