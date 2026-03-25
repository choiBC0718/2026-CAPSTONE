// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_ItemInteraction.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"


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
	/*
	if (ProgressMID)
	{
		ProgressMID->SetScalarParameterValue(FName("Percent"), Progress);
	}
	*/
	if (ProgressBar)
	{
		ProgressBar->SetPercent(Progress);
	}
}

void UCAP_ItemInteraction::SetInteractKeyText(const FString& KeyName)
{
	if (KeyIconDataTable)
	{
		FName RowName = FName(*KeyName);
		FKeyIconRow* Row = KeyIconDataTable->FindRow<FKeyIconRow>(RowName,"");
		if (Row && Row->Icon)
		{
			if (EquipIconImg)
			{
				EquipIconImg->SetBrushFromTexture(Row->Icon);
				EquipIconImg->SetVisibility(ESlateVisibility::Visible);
			}
			if (DisassembleIconImg)
			{
				DisassembleIconImg->SetBrushFromTexture(Row->Icon);
				DisassembleIconImg->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}
