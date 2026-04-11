// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_SynergySimulSlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCAP_SynergySimulSlotWidget::InitSlot(class UTexture2D* Icon, int32 OriginalCount, int32 NewCount)
{
	if (SynergyIcon && Icon)
		SynergyIcon->SetBrushFromTexture(Icon);

	if (CountText)
	{
		if (OriginalCount == NewCount)
		{
			CountText->SetText(FText::AsNumber(NewCount));
			CountText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
		else
		{
			FString DiffString = FString::Printf(TEXT("%d ▶ %d"), OriginalCount, NewCount);
			CountText->SetText(FText::FromString(DiffString));

			if (NewCount > OriginalCount)
			{
				CountText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
			}
			else
			{
				CountText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
			}
		}
	}
}
