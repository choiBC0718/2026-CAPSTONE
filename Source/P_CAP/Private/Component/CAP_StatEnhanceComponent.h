// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "CAP_StatEnhanceComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCAP_StatEnhanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCAP_StatEnhanceComponent();

	virtual void BeginPlay() override;
	
	int32 GetStatEnhanceLevel(FName RowName);
	bool UpgradeStatEnhance(FName RowName, int32 MaxLevel);
	void LoadEnhanceData(const TMap<FName, int32>& SavedData);

private:
	UPROPERTY()
	TMap<FName, int32> EnhancedStatLevels;
	UPROPERTY()
	TMap<FName, FActiveGameplayEffectHandle> ActiveGEHandles;
	UPROPERTY()
	UDataTable* StatEnhanceDT;

	void ApplyStatEnhanceGE(FName RowName, int32 Level);
};
