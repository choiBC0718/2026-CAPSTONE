// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_ValueGauge.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UCAP_ValueGauge::NativeConstruct()
{
	Super::NativeConstruct();

	ProgressBar->SetFillColorAndOpacity(BarColor);
	ProgressBar->SetVisibility(bProgressBarVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	//ValueText->SetFont(ValueTextFont);
	ValueText->SetVisibility(bValueTextVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UCAP_ValueGauge::SetAndBoundToGameplayAttribute(class UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute)
{
	if (ASC)
	{
		bool bFound;
		float Value = ASC->GetGameplayAttributeValue(Attribute, bFound);
		float MaxValue = ASC->GetGameplayAttributeValue(MaxAttribute, bFound);
		if (bFound)
		{
			SetValue(Value, MaxValue);
		}

		ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UCAP_ValueGauge::ValueChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UCAP_ValueGauge::MaxValueChanged);
	}
}

void UCAP_ValueGauge::SetValue(float NewValue, float NewMaxValue)
{
	CachedValue = NewValue;
	CachedMaxValue = NewMaxValue;

	if (NewMaxValue == 0)
		return;

	float NewPercent = NewValue / NewMaxValue;
	ProgressBar->SetPercent(NewPercent);
	FNumberFormattingOptions FormatOps = FNumberFormattingOptions().SetMaximumFractionalDigits(0);

	ValueText->SetText(
		FText::Format(
			FTextFormat::FromString("{0} / {1}"),
			FText::AsNumber(NewValue, &FormatOps),
			FText::AsNumber(NewMaxValue, &FormatOps)));
}

void UCAP_ValueGauge::ValueChanged(const FOnAttributeChangeData& ChangedData)
{
	SetValue(ChangedData.NewValue, CachedMaxValue);
}

void UCAP_ValueGauge::MaxValueChanged(const FOnAttributeChangeData& ChangedData)
{
	SetValue(CachedValue, ChangedData.NewValue);
}
