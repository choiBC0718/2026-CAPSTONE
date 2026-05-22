// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CAP_StatEnhanceTypes.generated.h"

USTRUCT(BlueprintType)
struct FStatModifierInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Enhance", meta=(Categories="Data.ItemStat"))
	FGameplayTag StatTag;

	// 레벨 별 강화 효과 증가량
	UPROPERTY(EditAnywhere, Category="Enhance")
	float Value = 0.f;
	UPROPERTY(EditAnywhere, Category="Enhance")
	bool bIsFixedValue = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPercentage = false;
};

USTRUCT(BlueprintType)
struct FStatEnhanceTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
	// 강화 이름 (ex. 물리 공격력 강화)
	UPROPERTY(EditAnywhere, Category="Enhance")
	FText DisplayName;
	// 강화 설명 (ex. 물리 공격력을 n만큼 상승시킨다.)
	UPROPERTY(EditAnywhere, Category="Enhance")
	FText Description;
	UPROPERTY(EditAnywhere, Category="Enhance")
	TSoftObjectPtr<class UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, Category="Enhance")
	int32 MaxLevel = 1;
	// 레벨 별 강화 비용 증가량
	UPROPERTY(EditAnywhere, Category="Enhance")
	int32 CostIncreaseRate =0;
	
	UPROPERTY(EditAnywhere, Category="Enhance")
	TArray<FStatModifierInfo> Modifiers;
};
