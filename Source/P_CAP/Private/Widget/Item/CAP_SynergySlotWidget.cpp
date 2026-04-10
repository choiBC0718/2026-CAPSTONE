// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_SynergySlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_SynergySlotData.h"

void UCAP_SynergySlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UCAP_SynergySlotData* SlotData = Cast<UCAP_SynergySlotData>(ListItemObject);
	if (!SlotData)
		return;

	if (SynergyNameText)
		SynergyNameText->SetText(SlotData->SynergyData.SynergyName);
	if (CurrentLevelText)
		CurrentLevelText->SetText(FText::AsNumber(SlotData->CurrentCount));

	if (Icon && !SlotData->SynergyData.SynergyIcon.IsNull())
	{
		if (UTexture2D* LoadedIcon = SlotData->SynergyData.SynergyIcon.LoadSynchronous())
		{
			Icon->SetBrushFromTexture(LoadedIcon);
		}
	}

	if (LevelRequireText)
	{
		FString ReqString = TEXT("");
		const TArray<FSynergyLevelData>& Levels = SlotData->SynergyData.SynergyLevels;

		for (int32 i = 0; i < Levels.Num(); i++)
		{
			ReqString += FString::FromInt(Levels[i].RequiredCount);
			if (i<Levels.Num() - 1)
			{
				ReqString += TEXT(" ▶ ");
			}
		}
		LevelRequireText->SetText(FText::FromString(ReqString));
	}
}
