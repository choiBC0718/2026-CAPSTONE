// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_CurrencyComponent.h"

UCAP_CurrencyComponent::UCAP_CurrencyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrencyMap.Add(ECurrencyType::Gold,0);
	CurrencyMap.Add(ECurrencyType::WeaponMaterial,0);
	CurrencyMap.Add(ECurrencyType::MagicStone,0);
}

void UCAP_CurrencyComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCAP_CurrencyComponent::AddCurrency(ECurrencyType Type, int32 Amount)
{
	if (Amount <=0)	return;

	int32& CurrentAmount = CurrencyMap.FindOrAdd(Type, 0);
	int32 OldAmount = CurrentAmount;
	
	CurrentAmount += Amount;

	if (OnCurrencyChanged.IsBound())
		OnCurrencyChanged.Broadcast(Type, OldAmount, CurrentAmount);
}

bool UCAP_CurrencyComponent::ConsumeCurrency(ECurrencyType Type, int32 Amount)
{
	if (Amount <=0)	return false;

	int32& CurrentAmount = CurrencyMap.FindOrAdd(Type, 0);

	// 잔액 부족
	if (CurrentAmount < Amount)
		return false; 

	int32 OldAmount = CurrentAmount;
	CurrentAmount -= Amount;

	if (OnCurrencyChanged.IsBound())
		OnCurrencyChanged.Broadcast(Type, OldAmount, CurrentAmount);

	return true;
}

int32 UCAP_CurrencyComponent::GetCurreny(ECurrencyType Type)
{
	if (const int32* AmountPtr = CurrencyMap.Find(Type))
		return *AmountPtr;
	return 0;
}

void UCAP_CurrencyComponent::SetCurrencyOverride(ECurrencyType Type, int32 Amount)
{
	int32& CurrentAmount = CurrencyMap.FindOrAdd(Type, 0);
	int32 OldAmount = CurrentAmount;
	
	CurrentAmount = FMath::Max(0, Amount);

	if (OldAmount != CurrentAmount && OnCurrencyChanged.IsBound())
		OnCurrencyChanged.Broadcast(Type, OldAmount, CurrentAmount);
}

