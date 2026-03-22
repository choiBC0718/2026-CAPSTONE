// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_WeaponDataAsset.generated.h"

UENUM(BlueprintType)
enum class EWeaponGrade : uint8
{
	Normal,
	Rare,
	Epic,
	Legendary
};


/**
 * 
 */
UCLASS()
class UCAP_WeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 무기 이름 */
	UPROPERTY(EditDefaultsOnly)
	FText WeaponName;
	/** 무기 기본 등급 */
	UPROPERTY(EditDefaultsOnly)
	EWeaponGrade DefaultGrade = EWeaponGrade::Normal;

	/** 무기의 기본 공격 */
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<class UCAP_GameplayAbility> BasicAttack;
	/** 무기의 고유 스킬 배열 */
	UPROPERTY(EditDefaultsOnly)
	TArray<TSoftClassPtr<class UCAP_GameplayAbility>> ActiveSkillArray;

	UPROPERTY(EditDefaultsOnly)
	class UDataTable* StatDataTable;	
	
	UPROPERTY(EditDefaultsOnly)
	TMap<EWeaponGrade, FName> GradeDataMap;
};
