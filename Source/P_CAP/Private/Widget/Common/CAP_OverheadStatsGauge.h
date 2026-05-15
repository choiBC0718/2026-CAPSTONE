// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_OverheadStatsGauge.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_OverheadStatsGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Stats")
	void ConfigureWithASC(class UAbilitySystemComponent* AbilitySystemComponent);

private:
	UPROPERTY(meta = (BindWidget))
	class UCAP_ValueGauge* HealthBar;
};
