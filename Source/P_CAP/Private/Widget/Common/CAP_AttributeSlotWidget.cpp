// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_AttributeSlotWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/TextBlock.h"


void UCAP_AttributeSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	NumberFormattingOptions.MaximumFractionalDigits = 0;
	
	APawn* OwnerPlayerPawn = GetOwningPlayerPawn();
	if (!OwnerPlayerPawn)
		return;

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPlayerPawn);
	if (OwnerASC)
	{
		bool bFound;
		float AttributeVal = OwnerASC->GetGameplayAttributeValue(Attribute, bFound);
		SetValue(AttributeVal);

		OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UCAP_AttributeSlotWidget::AttributeChanged);
	}
	
}

void UCAP_AttributeSlotWidget::SetValue(float NewValue)
{
	AttributeText->SetText(FText::AsNumber(NewValue, &NumberFormattingOptions));
}

void UCAP_AttributeSlotWidget::AttributeChanged(const FOnAttributeChangeData& Data)
{
	SetValue(Data.NewValue);
}
