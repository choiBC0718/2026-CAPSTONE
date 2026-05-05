// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_AttributeSlotWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/TextBlock.h"
#include "GAS/CAP_AbilitySystemComponent.h"


void UCAP_AttributeSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	NumberFormattingOptions.MaximumFractionalDigits = 0;
	
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn());
	if (!Player)
		return;

	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);
	if (OwnerASC)
	{
		UpdateAttributeValue();
		OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UCAP_AttributeSlotWidget::AttributeChanged);
	}
	if (AttributeNameText)
	{
		AttributeNameText->SetText(FText(AttributeName));
	}
}

void UCAP_AttributeSlotWidget::UpdateAttributeValue()
{
	if (!OwnerASC || !AttributeValueText)
		return;

	float CurrentVal = OwnerASC->GetNumericAttribute(Attribute);
	FString ResultString = TEXT("");

	switch (AttributeDisplay)
	{
		case EAttributeDisplay::RawValue:
			ResultString = FString::Printf(TEXT("%d"), FMath::RoundToInt(CurrentVal));
			break;
		case EAttributeDisplay::Percentage:
			ResultString = FString::Printf(TEXT("%.1f%%"), CurrentVal);
			break;
		case EAttributeDisplay::Multiplier:
			ResultString = FString::Printf(TEXT("%.2fx"), CurrentVal);
			break;
		case EAttributeDisplay::RatioFromBase:
			{
				float BaseVal = OwnerASC->GetNumericAttributeBase(Attribute);
				if (FMath::IsNearlyZero(BaseVal)) BaseVal = 1.f;
				float Percentage = (CurrentVal / BaseVal) * 100.f;
				ResultString = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Percentage));
			}
			break;
	}
	
	AttributeValueText->SetText(FText::FromString(ResultString));
}

void UCAP_AttributeSlotWidget::AttributeChanged(const FOnAttributeChangeData& Data)
{
	UpdateAttributeValue();
}
