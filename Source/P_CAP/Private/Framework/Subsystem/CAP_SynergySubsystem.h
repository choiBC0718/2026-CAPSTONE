// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/CAP_SynergyDataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CAP_SynergySubsystem.generated.h"


/**
 * 
 */
UCLASS()
class UCAP_SynergySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY()
	TMap<FGameplayTag, TSoftObjectPtr<UCAP_SynergyDataAsset>> SynergyMap;
};
