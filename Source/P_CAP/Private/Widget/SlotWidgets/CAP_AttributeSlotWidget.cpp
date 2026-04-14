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
		UpdatePercentage();
		OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UCAP_AttributeSlotWidget::AttributeChanged);
	}
	if (AttributeNameText)
	{
		AttributeNameText->SetText(FText(AttributeName));
	}
}

void UCAP_AttributeSlotWidget::UpdatePercentage()
{
	if (!OwnerASC)
		return;

	float CurrentVal = OwnerASC->GetNumericAttribute(Attribute);
	float BaseVal = OwnerASC->GetNumericAttributeBase(Attribute);
	if (FMath::IsNearlyZero(BaseVal))
	{
		BaseVal = 1.f;
	}
	float Percentage = (CurrentVal / BaseVal) * 100.f;

	FString Str = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Percentage));
	if (AttributeValueText)
	{
		AttributeValueText->SetText(FText::FromString(Str));
	}
}

void UCAP_AttributeSlotWidget::AttributeChanged(const FOnAttributeChangeData& Data)
{
	UpdatePercentage();
}
