// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CAP_GameplayAbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None				UMETA(DisplayName = "None"),
	BasicAttack			UMETA(DisplayName = "BasicAttack"),
	Skill1				UMETA(DisplayName = "Skill1"),
	Skill2				UMETA(DisplayName = "Skill2"),
	ActiveItem			UMETA(DisplayName = "ActiveItem"),
	Dodge				UMETA(DisplayName = "Dodge"),
	SwapWeapon			UMETA(DisplayName = "SwapWeapon")
};

UENUM(BlueprintType)
enum class EItemGrade : uint8
{
	Normal			UMETA(DisplayName = "일반"),
	Rare			UMETA(DisplayName = "레어"),
	Epic			UMETA(DisplayName = "에픽"),
	Legendary		UMETA(DisplayName = "레전더리")
};

USTRUCT(BlueprintType)
struct FBaseStatRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)		float BaseMaxHealth=0.f;
	UPROPERTY(EditAnywhere)		float BasePhysicalDamage=0.f;
	UPROPERTY(EditAnywhere)		float BasePhysicalPenetration=0.f;
	UPROPERTY(EditAnywhere)		float BaseMagicalDamage=0.f;
	UPROPERTY(EditAnywhere)		float BaseMagicalPenetration=0.f;
	UPROPERTY(EditAnywhere)		float BasePhysicalArmor=0.f;
	UPROPERTY(EditAnywhere)		float BaseMagicalArmor=0.f;
	UPROPERTY(EditAnywhere)		float BaseCriticalChance=0.f;
	UPROPERTY(EditAnywhere)		float BaseCriticalDamage=0.f;
	UPROPERTY(EditAnywhere)		float BaseMoveSpeed=0.f;
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