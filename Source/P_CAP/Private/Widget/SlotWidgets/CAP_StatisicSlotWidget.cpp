// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_StatisicSlotWidget.h"

#include "Components/TextBlock.h"

void UCAP_StatisicSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (TitleText)
		TitleText->SetText(StatisticTitle);
}

void UCAP_StatisicSlotWidget::SetStatisticData(const FText& InValue)
{
	ValueText->SetText(InValue);
}
