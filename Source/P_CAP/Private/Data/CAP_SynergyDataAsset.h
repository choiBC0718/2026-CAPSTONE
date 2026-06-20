// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_EquipItemEffectTypes.h"
#include "Engine/DataAsset.h"
#include "CAP_SynergyDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_SynergyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Synergy"),AssetRegistrySearchable)
	FGameplayTag SynergyTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText SynergyName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SynergyIcon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Synergy")
	TArray<FSynergyLevelData> SynergyLevels;
};
