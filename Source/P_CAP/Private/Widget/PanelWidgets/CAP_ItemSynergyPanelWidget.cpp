// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_ItemSynergyPanelWidget.h"

#include "Widget/Common/CAP_SynergyListView.h"
#include "Data/CAP_SynergySlotData.h"
#include "Framework/Subsystem/CAP_SynergySubsystem.h"

void UCAP_ItemSynergyPanelWidget::RefreshSynergyList(const TMap<FGameplayTag, int32>& CurrentCounts)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance())
		return;
	UCAP_SynergySubsystem* SynergySubsystem = World->GetGameInstance()->GetSubsystem<UCAP_SynergySubsystem>();
	if (!SynergySubsystem || !SynergyListView)
		return;
	
	SynergyListView->ClearListItems();
	// 많이 활성화된 순으로 정렬위한 배열
	TArray<UCAP_SynergySlotData*> SynergySlots;
	
	for (const TPair<FGameplayTag, int32>& CountPair : CurrentCounts)
	{
		FGameplayTag Tag = CountPair.Key;
		int32 Count = CountPair.Value;
		if (Count > 0 && SynergySubsystem->SynergyMap.Contains(Tag))
		{
			if (UCAP_SynergyDataAsset* SynergyDA = SynergySubsystem->SynergyMap[Tag].LoadSynchronous())
			{
				UCAP_SynergySlotData* NewData = NewObject<UCAP_SynergySlotData>(this);
				NewData->SynergyTag = Tag;           // 식별용 태그
				NewData->SynergyDA = SynergyDA;      // 가벼운 에셋 포인터
				NewData->CurrentCount = Count;       // 현재 활성화 스택 수

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
