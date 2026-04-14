// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CAP_SynergyTypes.generated.h"

USTRUCT(BlueprintType)
struct FSynergyLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 RequiredCount = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UGameplayEffect> SynergyEffect;
};


USTRUCT(BlueprintType)
struct FSynergyDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Synergy"))
	FGameplayTag SynergyTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText SynergyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SynergyIcon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSynergyLevelData> SynergyLevels;
};