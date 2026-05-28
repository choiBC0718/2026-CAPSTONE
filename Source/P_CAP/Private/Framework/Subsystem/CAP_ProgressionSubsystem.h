// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CAP_ProgressionSubsystem.generated.h"

// 무기 Instance 1개 데이터
USTRUCT(BlueprintType)
struct FWeaponSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	class UCAP_WeaponDataAsset* WeaponDA = nullptr;
	UPROPERTY()
	EItemGrade CurrentGrade = EItemGrade::Normal;
	UPROPERTY()
	TArray<FName> GrantedSkillRowNames;
};
// WeaponComponent 전체 데이터
USTRUCT(BlueprintType)
struct FWeaponComponentSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FWeaponSaveData> HeldWeapons;
	UPROPERTY()
	int32 EquippedWeaponIdx=0;
};

// InventoryComponent 데이터
USTRUCT(BlueprintType)
struct FInventorySaveData
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<class UCAP_ItemDataBase*> HeldItemDAs;
};

USTRUCT(BlueprintType)
struct FCurrencySaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ECurrencyType, int32> SavedCurrencies;
};

USTRUCT(BlueprintType)
struct FPlayerProgressionData
{
	GENERATED_BODY()

	// 데이터가 유효한지(이전 맵에서 넘어온 게 맞는지) 체크하는 플래그
	UPROPERTY()
	bool bIsValid = false;

	UPROPERTY()
	float CurrentHealth = -1.0f;
	UPROPERTY()
	FWeaponComponentSaveData WeaponData;
	UPROPERTY()
	FInventorySaveData InventoryData;
	UPROPERTY()
	FCurrencySaveData CurrencyData;
};

/**
 * 스테이지 간 레벨 이동이 일어날 때, 현재 캐릭터의 정보를 받고 일시 저장 + 제공 해줄 클래스
 */
UCLASS()
class UCAP_ProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 레벨 이동 전 CurrentRunData에 새로 저장
	void SavePlayerProgression(const FPlayerProgressionData& InData);
	// 레벨 이동 후 CurrenRunData에서 꺼내가 데이터 덮어씌움
	bool LoadPlayerProgression(FPlayerProgressionData& OutData);
	// 캐릭터 죽었을 때 정보 폐기
	void ClearProgression();

private:
	UPROPERTY()
	FPlayerProgressionData CurrentRunData;
};
