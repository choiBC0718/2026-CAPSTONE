// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CAP_StatEnhanceTypes.generated.h"

USTRUCT(BlueprintType)
struct FStatEnhanceTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Enhance", meta=(Categories="Data.ItemStat"))
	FGameplayTag StatTag;
	
	UPROPERTY(EditAnywhere, Category="Enhance")
	FText DisplayName;
	UPROPERTY(EditAnywhere, Category="Enhance")
	FText Description;
	UPROPERTY(EditAnywhere, Category="Enhance")
	TSoftObjectPtr<class UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, Category="Enhance")
	int32 MaxLevel = 1;
	UPROPERTY(EditAnywhere, Category="Enhance")
	TArray<int32> CostPerLevel;
	UPROPERTY(EditAnywhere, Category="Enhance")
	TArray<int32> ValuePerLevel;
};
