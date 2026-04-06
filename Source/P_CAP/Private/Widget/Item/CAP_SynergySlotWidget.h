// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "CAP_SynergySlotWidget.generated.h"

/**
 * 시너지 1개를 나타낼 슬롯 위젯 클래스
 */
UCLASS()
class UCAP_SynergySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshSynergy(const TMap<FGameplayTag, int32>& CurrentSynergies);

protected:
};
