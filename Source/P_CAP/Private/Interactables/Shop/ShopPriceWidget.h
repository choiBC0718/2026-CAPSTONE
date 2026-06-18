// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "ShopPriceWidget.generated.h"

UCLASS()
class UShopPriceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Shop")
	void SetOfferDisplayData(ECurrencyType InCurrencyType, int32 InPriceAmount, bool bInPurchased);

	UPROPERTY(BlueprintReadOnly, Category="Shop")
	ECurrencyType CurrencyType = ECurrencyType::Gold;

	UPROPERTY(BlueprintReadOnly, Category="Shop")
	int32 PriceAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Shop")
	bool bPurchased = false;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Shop")
	void OnOfferChanged();
};
