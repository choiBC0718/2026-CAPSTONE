// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_SynergySlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_SynergyDataAsset.h"
#include "Data/CAP_SynergySlotData.h"

void UCAP_SynergySlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UCAP_SynergySlotData* SlotData = Cast<UCAP_SynergySlotData>(ListItemObject);
	if (!SlotData || !SlotData->SynergyDA)
		return;

	if (SynergyNameText)
		SynergyNameText->SetText(SlotData->SynergyDA->SynergyName);
	if (CurrentLevelText)
		CurrentLevelText->SetText(FText::AsNumber(SlotData->CurrentCount));

	if (Icon && !SlotData->SynergyDA->SynergyIcon.IsNull())
		Icon->SetBrushFromTexture(SlotData->SynergyDA->SynergyIcon.LoadSynchronous());
	
	if (LevelRequireText)
	{
		FString ReqString = TEXT("");
		const TArray<FSynergyLevelData>& Levels = SlotData->SynergyDA->SynergyLevels;

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
