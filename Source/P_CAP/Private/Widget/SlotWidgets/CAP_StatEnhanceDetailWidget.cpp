// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_StatEnhanceDetailWidget.h"

#include "Components/TextBlock.h"

void UCAP_StatEnhanceDetailWidget::UpdateDetailInfo(const FText& EnhanceName, const FText& Description, int32 MaxLv, int32 CurrentLv)
{
	if (DisplayName)		DisplayName->SetText(EnhanceName);
	if (DescriptionText)	DescriptionText->SetText(Description);
	if (LevelText)			LevelText->SetText(FText::FromString(FString::Printf(TEXT("Lv. %d/%d"),CurrentLv,MaxLv)));
}
