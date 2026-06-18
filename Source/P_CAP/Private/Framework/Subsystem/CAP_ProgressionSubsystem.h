// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CAP_ProgressionSubsystem.generated.h"

// 이번판 통계 데이터
USTRUCT(BlueprintType)
struct FRunStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float RunStartTime = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float TotalPlayTime = 0.f;

	UPROPERTY(BlueprintReadOnly)
	int32 EnemiesDefeated=0;
	UPROPERTY(BlueprintReadOnly)
	float TotalDamageDeal = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float MaxDamageDeal=0.f;
	UPROPERTY(BlueprintReadOnly)
	float TotalDamageTaken=0.f;
	UPROPERTY(BlueprintReadOnly)
	float TotalHealing = 0.f;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalGetMagicStone=0;
	UPROPERTY(BlueprintReadOnly)
	int32 TotalGetGold=0;
	UPROPERTY(BlueprintReadOnly)
	int32 TotalGetWeaponMaterial=0;
};

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
	float BonusMaxHealth = 0.f;
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
	void AddBonusMaxHealth(float Amount);
	float GetBonusMaxHealth() const { return CurrentRunData.BonusMaxHealth; }
	// 캐릭터 장비, 아이템 정보 초기화 (레벨 변환 후 호출용)
	void ClearProgression();

	const FRunStatistics& GetRunStatistics() const {return CurrentRunStats;}

	void StartRunTimer(float CurrentTime);
	void EndRunTimer(float CurrentTime);

	void AddDamageDeal(float Damage);
	void AddDamageTaken(float Damage);
	void AddHealing(float Healing);
	void AddEnemyDefeated();
	void AddCurrencyCnt(ECurrencyType Type, int32 Amount);
	// 통계 데이터 초기화 (마을로 돌아가는 경우에 호출)
	void ClearRunStats();
private:
	UPROPERTY()
	FPlayerProgressionData CurrentRunData;
	UPROPERTY()
	FRunStatistics CurrentRunStats;
};
