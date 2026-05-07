// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_CurrencyComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Framework/CAP_RewardSettings.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AttributeSet.h"

UCAP_CurrencyComponent::UCAP_CurrencyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCAP_CurrencyComponent::BeginPlay()
{
	Super::BeginPlay();

	const UCAP_RewardSettings* RewardSetting = GetDefault<UCAP_RewardSettings>();
	if (!RewardSetting->DisassembleRewardDT.IsNull())
		LoadedRewardDisassembleDT = RewardSetting->DisassembleRewardDT.LoadSynchronous();
}

void UCAP_CurrencyComponent::ProcessDisassembleReward(EItemGrade Grade, ECurrencyType Type)
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwner());
	if (!Player)
		return;

	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player));
	if (!ASC)
		return;
	
	if (LoadedRewardDisassembleDT)
	{
		FName GradeName = GetRowNameFromGrade(Grade);
		if (const FDisassembleRewardRow* RewardRow = LoadedRewardDisassembleDT->FindRow<FDisassembleRewardRow>(GradeName,""))
		{
			float BonusMul = ASC->GetNumericAttribute(UCAP_AttributeSet::GetDisassembleBonusMultiplierAttribute());
			int32 BaseAmount = Type==ECurrencyType::WeaponMaterial ? RewardRow->WeaponRewardAmount : RewardRow->ItemRewardAmount;
			int32 FinalAmount = FMath::RoundToInt(BaseAmount * (1.f + BonusMul));

			AddCurrency(Type, FinalAmount);
		}
	}
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

FName UCAP_CurrencyComponent::GetRowNameFromGrade(EItemGrade Grade)
{
	switch (Grade)
	{
	case EItemGrade::Normal:		return FName("Normal");
	case EItemGrade::Rare:			return FName("Rare");
	case EItemGrade::Epic:			return FName("Epic");
	case EItemGrade::Legendary:		return FName("Legendary");
	default:						return FName();
	}
}

