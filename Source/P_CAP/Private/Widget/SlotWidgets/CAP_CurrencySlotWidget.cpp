// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_CurrencySlotWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCAP_CurrencySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn());
	if (Player)
	{
		if (UCAP_CurrencyComponent* CurrencyComp = Player->GetCurrencyComponent())
		{
			CurrencyComp->OnCurrencyChanged.AddDynamic(this, &UCAP_CurrencySlotWidget::OnCurrencyChanged);

			if (CurrencyText)
			{
				int32 CurrentCurrency = CurrencyComp->GetCurreny(Currency);
				SetCurrencyText(CurrentCurrency);
			}
		}
	}
	if (CurrencyIcon && !CurrencyIconTexture)
		CurrencyIcon->SetVisibility(ESlateVisibility::Collapsed);
	else if (CurrencyIcon && CurrencyIconTexture)
		CurrencyIcon->SetBrushFromTexture(CurrencyIconTexture);
}

void UCAP_CurrencySlotWidget::OnCurrencyChanged(ECurrencyType CurrencyType, int32 OldAmount, int32 NewAmount)
{
	if (CurrencyType == Currency)
	{
		SetCurrencyText(NewAmount);
	}
}

void UCAP_CurrencySlotWidget::SetCurrencyText(int32 Amount)
{
	FNumberFormattingOptions Opts;
	Opts.UseGrouping = true;
	FText FormatText = FText::AsNumber(Amount, &Opts);
		
	CurrencyText->SetText(FormatText);
}
