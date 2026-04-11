// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_AbilitySlot.h"

#include "Components/Image.h"
#include "Data/CAP_AbilitySlotData.h"

void UCAP_AbilitySlot::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UCAP_AbilitySlotData* SlotData = Cast<UCAP_AbilitySlotData>(ListItemObject);
	if (SlotData && Icon)
	{
		UTexture2D* LoadedIcon = SlotData->SkillIcon.LoadSynchronous();
		if (LoadedIcon)
		{
			Icon->SetBrushFromTexture(LoadedIcon);
		}
	}
}
