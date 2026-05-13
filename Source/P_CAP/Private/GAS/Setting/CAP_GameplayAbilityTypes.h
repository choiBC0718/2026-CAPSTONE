// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	SwapWeapon			UMETA(DisplayName = "SwapWeapon"),
	Confirm				UMETA(DisplayName = "Confirm"),
	Cancel				UMETA(DisplayName = "Cancel"),
};

UENUM(BlueprintType)
enum class EItemGrade : uint8
{
	Normal			UMETA(DisplayName = "일반"),
	Rare			UMETA(DisplayName = "레어"),
	Epic			UMETA(DisplayName = "에픽"),
	Legendary		UMETA(DisplayName = "레전더리")
};


UENUM(BlueprintType)
enum class ECurrencyType : uint8
{
	Gold			UMETA(DisplayName = "Gold (골드)"),
	WeaponMaterial	UMETA(DisplayName = "WeaponMaterial (무기 강화재료)"),
	MagicStone		UMETA(DisplayName = "MagicStone (마석)"),
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
	UPROPERTY(EditAnywhere)		float BaseAttackSpeed=0.f;
	UPROPERTY(EditAnywhere)		float BaseMoveSpeed=0.f;
	UPROPERTY(EditAnywhere)		float BaseCooldownMultiplier=0.f;
	UPROPERTY(EditAnywhere)		float BaseWeaponSwapCooldownMultiplier=0.f;
};