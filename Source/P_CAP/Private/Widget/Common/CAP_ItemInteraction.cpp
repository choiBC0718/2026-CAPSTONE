// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_ItemInteraction.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"


void UCAP_ItemInteraction::NativeConstruct()
{
	Super::NativeConstruct();
	if (InteractProgressImage)
	{
		ProgressMID = InteractProgressImage->GetDynamicMaterial();
	}
}

void UCAP_ItemInteraction::SetInteractionUIVisibility(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	if (bVisible)
	{
		UpdateInteractProgress(0.f);
	}
}

void UCAP_ItemInteraction::UpdateInteractProgress(float Progress)
{
	if (ProgressMID)
	{
		ProgressMID->SetScalarParameterValue(FName("Percent"), Progress);
	}
}

void UCAP_ItemInteraction::SetInteractKeyText(const FString& KeyName)
{
	if (KeyText)
	{
		KeyText->SetText(FText::FromString(KeyName));
	}
}
