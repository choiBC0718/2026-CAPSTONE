// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "CAP_SynergySlotWidget.generated.h"

/**
 * InventoryTabWidget에 들어갈 패널 (소유한 아이템 시너지)의 요소
 */
UCLASS()
class UCAP_SynergySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshSynergy(const TMap<FGameplayTag, int32>& CurrentSynergies);

protected:
};
