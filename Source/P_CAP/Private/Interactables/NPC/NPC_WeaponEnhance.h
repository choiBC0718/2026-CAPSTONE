// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/NPC/CAP_WorldNPC.h"
#include "NPC_WeaponEnhance.generated.h"

UENUM(BlueprintType)
enum class EWeaponUpgradeResult : uint8
{
	Default,
	Success,
	InsufficientCurrency,
	MaxGradeReached,
	ConfirmMode,
	Error,
};

/**
 * 
 */
UCLASS()
class ANPC_WeaponEnhance : public ACAP_WorldNPC
{
	GENERATED_BODY()

public:
	virtual ENPCActionResult ExecuteSpecialAction(AActor* Actor) override;

	EWeaponUpgradeResult TryUpgradeWeapon(class ACAP_PlayerCharacter* Player);
	FText GetDialogueText(EWeaponUpgradeResult Result, int32 Cost = -1) const;

	UPROPERTY(EditAnywhere, Category="Enhance|Data")
	TMap<EItemGrade, int32> UpgradeCostMap;
protected:

	// 강화 직전 떠드는 대사
	UPROPERTY(EditDefaultsOnly, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> DefaultDialoguePool;
	// 강화 완료 대사
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> SuccessDialoguePool;
	// 재료 불충분 대사
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> InsufficientDialoguePool;
	// 이미 최대 강화 대사
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> OnMaxLevelDialoguePool;
	// 강화 진행 선택 시 {Cost} 얼마 든다는 대사
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> ConfirmDialoguePool;
};
