// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Shop/ShopPriceWidget.h"

void UShopPriceWidget::SetOfferDisplayData(ECurrencyType InCurrencyType, int32 InPriceAmount, bool bInPurchased)
{
	CurrencyType = InCurrencyType;
	PriceAmount = InPriceAmount;
	bPurchased = bInPurchased;

	OnOfferChanged();
}
