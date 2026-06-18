// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_SynergyToolTipWidget.h"

#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Data/CAP_EquipItemEffectTypes.h"

void UCAP_SynergyToolTipWidget::SetupToolTip(const struct FSynergyDataTable* SynergyDataTable)
{
	if (!SynergyDataTable)
		return;

	if (SynergyNameText)
		SynergyNameText->SetText(SynergyDataTable->SynergyName);

	if (SynergyEffectsText)
	{
		FString EffectsStr = TEXT("");

		for (const FSynergyLevelData& LevelData : SynergyDataTable->SynergyLevels)
		{
			EffectsStr += FString::Printf(TEXT("[%d레벨]\n%s\n"),LevelData.RequiredCount, *LevelData.LevelDescription.ToString());
		}

		SynergyEffectsText->SetText(FText::FromString(EffectsStr));
	}
	if (SynergyIcon)
		SynergyIcon->SetBrushFromTexture(SynergyDataTable->SynergyIcon.LoadSynchronous());
}
