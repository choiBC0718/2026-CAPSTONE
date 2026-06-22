// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/NPC/CAP_WorldNPC.h"
#include "NPC_SkillReroll.generated.h"

/**
 * 
 */
UCLASS()
class ANPC_SkillReroll : public ACAP_WorldNPC
{
	GENERATED_BODY()

public:
	virtual ENPCActionResult ExecuteSpecialAction(AActor* Actor) override;

	FText GetDialogueText(EEnhanceResult Result, int32 Cost = -1) const;
	EEnhanceResult TryRerollSkill(class ACAP_PlayerCharacter* Player);

	UPROPERTY(EditAnywhere, Category="Enhance|Data")
	TMap<EItemGrade, int32> RerollCostMap;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> DefaultDialoguePool;
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> SuccessDialoguePool;
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> InsufficientDialoguePool;
	// 강화 진행 선택 시 {Cost} 얼마 든다는 대사
	UPROPERTY(EditAnywhere, Category="Enhance|Dialogue", meta=(MultiLine="true"))
	TArray<FText> ConfirmDialoguePool;
};
