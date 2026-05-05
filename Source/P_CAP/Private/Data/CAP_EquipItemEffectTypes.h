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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 RequiredCount = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UGameplayEffect> SynergyEffect;
};


USTRUCT(BlueprintType)
struct FSynergyDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Synergy"))
	FGameplayTag SynergyTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText SynergyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SynergyIcon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSynergyLevelData> SynergyLevels;
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
	/** 무기별 회피기 최대 연속 사용 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxDodgeCount = 2;
	/** 무기 업그레이드 가능한지 (레전더리라면 false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUpgradeable = true;
};

USTRUCT(BlueprintType)
struct FDisassembleRewardRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	ECurrencyType WeaponCurrencyType = ECurrencyType::WeaponMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int32 WeaponRewardAmount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	ECurrencyType ItemCurrencyType = ECurrencyType::Gold;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	int32 ItemRewardAmount = 0;
};