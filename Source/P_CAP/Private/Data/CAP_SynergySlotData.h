// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_SynergyTypes.h"
#include "CAP_SynergySlotData.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_SynergySlotData : public UObject
{
	GENERATED_BODY()

public:
	FSynergyDataTable SynergyData;
	int32 CurrentCount=0;
};
