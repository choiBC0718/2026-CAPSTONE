// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_SynergyToolTipWidget.h"

#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "Data/CAP_SynergyDataAsset.h"

void UCAP_SynergyToolTipWidget::SetupToolTip(UCAP_SynergyDataAsset* SynergyDA)
{
	if (!SynergyDA)
		return;

	if (SynergyNameText)
		SynergyNameText->SetText(SynergyDA->SynergyName);

	if (SynergyEffectsText)
	{
		FString EffectsStr = TEXT("");

		for (const FSynergyLevelData& LevelData : SynergyDA->SynergyLevels)
		{
			EffectsStr += FString::Printf(TEXT("[%d레벨]\n%s\n"),LevelData.RequiredCount, *LevelData.LevelDescription.ToString());
		}

		SynergyEffectsText->SetText(FText::FromString(EffectsStr));
	}
	if (SynergyIcon)
		SynergyIcon->SetBrushFromTexture(SynergyDA->SynergyIcon.LoadSynchronous());
}
