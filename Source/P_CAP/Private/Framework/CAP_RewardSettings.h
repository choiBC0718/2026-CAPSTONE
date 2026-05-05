// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "Engine/DeveloperSettings.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_RewardSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="아이템 분해 보상 설정"))
class UCAP_RewardSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TMap<EItemGrade, FDisassembleRewardRow> DisassembleRewardMap;
};
