// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_ItemSynergyPanelWidget.h"

#include "CAP_SynergyListView.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_SynergySlotData.h"
#include "Data/CAP_SynergyTypes.h"

void UCAP_ItemSynergyPanelWidget::RefreshSynergyList(const TMap<FGameplayTag, int32>& CurrentCounts, const TMap<FGameplayTag, FSynergyDataTable*>& SynergyCache)
{
	if (!SynergyListView || SynergyCache.IsEmpty())
		return;
	
	SynergyListView->ClearListItems();

	// 많이 활성화된 순으로 정렬위한 배열
	TArray<UCAP_SynergySlotData*> SynergySlots;
	for (const TPair<FGameplayTag, int32>& CountPair : CurrentCounts)
	{
		FGameplayTag Tag = CountPair.Key;
		int32 Count = CountPair.Value;
		if (Count > 0)
		{
			if (FSynergyDataTable* FoundRow = SynergyCache.FindRef(Tag))
			{
				UCAP_SynergySlotData* NewData = NewObject<UCAP_SynergySlotData>(this);
				NewData->SynergyData = *FoundRow;
				NewData->CurrentCount = Count;

				SynergySlots.Add(NewData);
			}
		}
	}

	// 정렬 로직
	SynergySlots.Sort([](const UCAP_SynergySlotData& A, const UCAP_SynergySlotData& B)
	{
		return A.CurrentCount > B.CurrentCount;
	});
	
	for (UCAP_SynergySlotData* Data : SynergySlots)
	{
		SynergyListView->AddItem(Data);
	}
}
