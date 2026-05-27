// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_CurrencySlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_CurrencySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;


private:
	UPROPERTY(meta = (BindWidget))
	class UImage* CurrencyIcon;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CurrencyText;

	UPROPERTY(EditAnywhere)
	ECurrencyType Currency = ECurrencyType::Gold;
	UPROPERTY(EditAnywhere)
	UTexture2D* CurrencyIconTexture;
	
	UFUNCTION()
	void OnCurrencyChanged(ECurrencyType CurrencyType, int32 OldAmount, int32 NewAmount);

	void SetCurrencyText(int32 Amount);
};
