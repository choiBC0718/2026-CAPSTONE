// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_EquipItemEffectTypes.generated.h"

USTRUCT(BlueprintType)
struct FSynergyLevelData
{
	GENERATED_BODY()

	// 해당 효과 발동에 필요한 태그 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 RequiredCount = 0;

	// 능력치 증가 시너지인 경우
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FStatModifier> StatModifiers;

	// 패시브 능력 시너지인 경우
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<class UCAP_ItemBehaviorBase>> GrantedBehaviors;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText LevelDescription;
};


/**무기 별 보너스 스탯 (등급별로 행 다르게 제작)*/
USTRUCT(BlueprintType)
struct FWeaponStatRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** 보너스 스탯 Map <스탯 태그, 수치> */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Data.Stat"))
	TMap<FGameplayTag, float> BonusStat;
};

USTRUCT(BlueprintType)
struct FDisassembleRewardRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon")
	ECurrencyType WeaponCurrencyType = ECurrencyType::WeaponMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int32 WeaponRewardAmount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Item")
	ECurrencyType ItemCurrencyType = ECurrencyType::Gold;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	int32 ItemRewardAmount = 0;
};