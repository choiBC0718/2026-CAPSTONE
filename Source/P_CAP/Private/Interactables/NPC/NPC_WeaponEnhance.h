// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/NPC/CAP_WorldNPC.h"
#include "NPC_WeaponEnhance.generated.h"

UENUM(BlueprintType)
enum class EWeaponUpgradeResult : uint8
{
	Success,
	InsufficientCurrency,
	MaxGradeReached,
	NoWeaponEquipped,
	Error
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

protected:
	UPROPERTY(EditAnywhere, Category="Enhance|Data")
	TMap<EItemGrade, int32> UpgradeCostMap;
	
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> SuccessDialoguePool;
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> FailDialoguePool;
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> OnMaxLevelDialoguePool;
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> NoWeaponDialoguePool;
};
