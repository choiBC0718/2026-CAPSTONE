// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_CurrencyPanelWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_CurrencyPanelWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	class UCAP_CurrencySlotWidget* GoldSlot;
	UPROPERTY(meta = (BindWidget))
	class UCAP_CurrencySlotWidget* WeaponMatSlot;
	UPROPERTY(meta = (BindWidget))
	class UCAP_CurrencySlotWidget* StoneSlot;
	
};
