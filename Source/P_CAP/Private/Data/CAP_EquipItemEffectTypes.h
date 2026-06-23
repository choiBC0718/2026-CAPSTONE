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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<class UCAP_ItemBehaviorBase*> GrantedBehaviors;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MultiLine=true))
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

UENUM(BlueprintType)
enum class ECompareOperator : uint8
{
	Equal,			// ==
	GreaterThan,	// >
	GreaterThanOrEqual, // >=
	LessThan,		// <
	LessThanOrEqual // <=
};

USTRUCT(BlueprintType)
struct FItemConditionData
{
	GENERATED_BODY()

	// 감시할 대상 스탯 (예: Health, Shield)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayAttribute MonitorAttribute;

	// 비교 연산자
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ECompareOperator Operator = ECompareOperator::LessThanOrEqual;

	// 비율(%) 계산 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsPercentage = false;

	// 비율 계산 시 분모가 될 최대치 스탯 (예: MaxHealth)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="bIsPercentage"))
	FGameplayAttribute MaxAttribute;

	// 발동 기준 수치 (비율이면 0.0 ~ 1.0, 아니면 절대값)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Threshold = 0.f;
};

USTRUCT(BlueprintType)
struct FItemActionData
{
	GENERATED_BODY()

	// 주입할 스탯 태그 및 수치 맵 (예: [Data.Stat.Armor : 30.0])
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, float> StatModifiers;

	// (선택) 조건 달성 시 1회성으로 터뜨릴 이벤트 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag TriggerEventTag;
};