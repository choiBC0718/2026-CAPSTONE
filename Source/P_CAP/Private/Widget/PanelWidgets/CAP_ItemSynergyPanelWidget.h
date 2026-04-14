// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "CAP_ItemSynergyPanelWidget.generated.h"

struct FSynergyDataTable;
/**
 * 
 */
UCLASS()
class UCAP_ItemSynergyPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshSynergyList(const TMap<FGameplayTag, int32>& CurrentCounts, const TMap<FGameplayTag, FSynergyDataTable*>& SynergyCache);
	
private:
	UPROPERTY(meta=(BindWidget))
	class UCAP_SynergyListView* SynergyListView;
};
