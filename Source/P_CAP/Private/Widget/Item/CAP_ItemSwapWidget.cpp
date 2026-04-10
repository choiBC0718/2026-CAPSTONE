// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_ItemSwapWidget.h"

#include "CAP_ItemDetailPanelWidget.h"
#include "CAP_ItemSlotWidget.h"
#include "CAP_SwapDetailPanelWIdget.h"
#include "CAP_SynergySimulSlotWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Character/Player/CAP_PlayerController.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Items/Item/CAP_InventoryComponent.h"
#include "Items/Item/CAP_ItemBase.h"
#include "Items/Item/CAP_ItemInstance.h"

void UCAP_ItemSwapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

void UCAP_ItemSwapWidget::InitSwapUI(class ACAP_PlayerCharacter* Player, class UCAP_ItemInstance* NewItemInst)
{
	if (!Player || !NewItemInst)
		return;

	NewItemToSwap = NewItemInst;
	UCAP_InventoryComponent* Inventory = Player->GetInventoryComponent();
	if (!Inventory || !ItemWrapBox || !ItemSlotWidgetClass)
		return;

	ItemWrapBox->ClearChildren();
	ItemSlots.Empty();

	const TArray<UCAP_ItemInstance*>& InventoryItems = Inventory->GetInventoryItems();
	for (int i=0; i<Inventory->GetCapacity(); i++)
	{
		UCAP_ItemInstance* ItemInst = InventoryItems.IsValidIndex(i) ? InventoryItems[i] : nullptr;
		UTexture2D* Icon = (ItemInst && ItemInst->GetItemDA()) ? ItemInst->GetItemDA()->ItemIcon.LoadSynchronous() : nullptr;

		UCAP_ItemSlotWidget* NewSlot = CreateWidget<UCAP_ItemSlotWidget>(GetOwningPlayer(), ItemSlotWidgetClass);
		if (NewSlot)
		{
			NewSlot->SetSlotNumber(i);
			NewSlot->InitSlot(ESlotItemType::Item, Icon,ItemInst);

			if (UWrapBoxSlot* WrapSlot = ItemWrapBox->AddChildToWrapBox(NewSlot))
			{
				WrapSlot->SetPadding(ItemSlotMargin);
			}
			
			NewSlot->OnLeftMouseClick.AddUObject(this, &UCAP_ItemSwapWidget::HandleSlotLeftClicked);
			ItemSlots.Add(NewSlot);
		}
	}
	InitNearbySlot();
	if (NewItemDetailPanel)
	{
		NewItemDetailPanel->UpdateDetailInfo(NewItemInst);
	}
	UpdateTopSynergyIcons();
	if (InformationText)
	{
		InformationText->SetText(FText::FromString(TEXT("방향키: 선택 | F: 교체 | ESC: 취소")));
	}
}

void UCAP_ItemSwapWidget::MoveSelection(FVector2D InputVal)
{
	if (!CurrentSelectedSlot)	return;

	UCAP_ItemSlotWidget* NextSlot = nullptr;

	if (InputVal.X >0)			NextSlot = CurrentSelectedSlot->RightSlot;
	else if (InputVal.X < 0)	NextSlot = CurrentSelectedSlot->LeftSlot;
	else if (InputVal.Y > 0)	NextSlot = CurrentSelectedSlot->UpSlot;
	else if (InputVal.Y < 0)	NextSlot = CurrentSelectedSlot->DownSlot;

	if (NextSlot)
	{
		HandleSlotLeftClicked(NextSlot);
	}
}

void UCAP_ItemSwapWidget::ConfirmSwap()
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn());
	ACAP_PlayerController* PC = Cast<ACAP_PlayerController>(GetOwningPlayer());
	if (CurrentSelectedSlot && CurrentSelectedSlot->SlotItemData && NewItemToSwap && Player)
	{
		UCAP_ItemInstance* OldItem = Cast<UCAP_ItemInstance>(CurrentSelectedSlot->SlotItemData);
		AActor* InteractActor = Player->GetInteractableActor();
		
		if (Player->GetInventoryComponent()->SwapItem(OldItem, NewItemToSwap))
		{
			if (InteractActor)
			{
				InteractActor->Destroy();
				Player->SetNearbyInteractable(nullptr);
				Player->UpdateInteractUI(false);
			}
			FVector SpawnLoc = Player->GetActorLocation() + FVector(0.f,0.f,50.f);
			FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLoc);
			
			ACAP_ItemBase* DroppedItem = GetWorld()->SpawnActorDeferred<ACAP_ItemBase>(ACAP_ItemBase::StaticClass(), SpawnTransform);
			if (DroppedItem)
			{
				DroppedItem->ItemInstance = OldItem;
				DroppedItem->ItemDA = OldItem->GetItemDA();
				DroppedItem->FinishSpawning(SpawnTransform);
				DroppedItem->DropItem();
			}
		}

		if (PC && PC->GetGameplayWidget())
		{
			PC->GetGameplayWidget()->DeactivateSwitcher();
			PC->SetPause(false);
		}
	}
}

void UCAP_ItemSwapWidget::HandleSlotLeftClicked(class UCAP_ItemSlotWidget* ClickedSlot)
{
	if (CurrentSelectedSlot && CurrentSelectedSlot != ClickedSlot)
	{
		CurrentSelectedSlot->SetSlotSelected(false);
	}
	if (ClickedSlot)
	{
		ClickedSlot->SetSlotSelected(true);
		CurrentSelectedSlot = ClickedSlot;

		if (OldItemDetailPanel)
		{
			OldItemDetailPanel->UpdateDetailInfo(ClickedSlot->SlotItemData);
		}
		UpdateTopSynergyIcons();
	}
}

void UCAP_ItemSwapWidget::InitNearbySlot()
{
	int ItemColumns = 3; 
	
	for (int i = 0; i < ItemSlots.Num(); i++)
	{
		int Row = i / ItemColumns;
		int Col = i % ItemColumns;

		// 왼쪽 슬롯
		if (Col > 0) ItemSlots[i]->LeftSlot = ItemSlots[i - 1];
		else ItemSlots[i]->LeftSlot = ItemSlots.IsValidIndex(i + (ItemColumns - 1)) ? ItemSlots[i + (ItemColumns - 1)] : nullptr;
		// 오른쪽 슬롯
		if (Col < ItemColumns - 1 && i + 1 < ItemSlots.Num()) ItemSlots[i]->RightSlot = ItemSlots[i + 1];
		else ItemSlots[i]->RightSlot = ItemSlots.IsValidIndex(i - Col) ? ItemSlots[i - Col] : nullptr;
		// 위 슬롯
		if (Row > 0) 
			ItemSlots[i]->UpSlot = ItemSlots[i - ItemColumns];
		else // 맨 윗줄이라면
			ItemSlots[i]->UpSlot = ItemSlots.IsValidIndex(Col) ? ItemSlots[Col] : (ItemSlots.Num() > 0 ? ItemSlots.Last() : nullptr);

		// 아래 슬롯
		if (Row < (ItemSlots.Num() - 1) / ItemColumns) 
			ItemSlots[i]->DownSlot = ItemSlots[i + ItemColumns];
		else // 맨 아랫줄이라면
			ItemSlots[i]->DownSlot = ItemSlots.IsValidIndex(Col) ? ItemSlots[Col] : (ItemSlots.Num() > 0 ? ItemSlots.Last() : nullptr);
	}
	
	// 첫 번째 강제 클릭 처리
	if (ItemSlots.Num() > 0) HandleSlotLeftClicked(ItemSlots[0]);
	else CurrentSelectedSlot = nullptr;
}

void UCAP_ItemSwapWidget::UpdateTopSynergyIcons()
{
	if (!TopSynergyScrollBox) return;
	TopSynergyScrollBox->ClearChildren();

	if (!SynergySimulSlotClass || !NewItemToSwap) return;

	ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player || !Player->GetInventoryComponent()) return;

	UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent();
	const TMap<FGameplayTag, FSynergyDataTable*>& SynergyCache = InvComp->GetSynergyDataCache();

	if (SynergyCache.IsEmpty()) return;

	// 인벤토리에서 현재 시너지 원본 복사
	TMap<FGameplayTag, int32> SimulatedCounts = InvComp->GetCurrentSynergyCounts();

	// [빼기] 버릴 아이템의 시너지 감소
	if (CurrentSelectedSlot && CurrentSelectedSlot->SlotItemData)
	{
		if (UCAP_ItemInstance* OldItemInst = Cast<UCAP_ItemInstance>(CurrentSelectedSlot->SlotItemData))
		{
			if (OldItemInst->GetItemDA())
			{
				FGameplayTag OldTag1 = OldItemInst->GetItemDA()->SynergyTag1;
				FGameplayTag OldTag2 = OldItemInst->GetItemDA()->SynergyTag2;

				if (OldTag1.IsValid() && SimulatedCounts.Contains(OldTag1)) SimulatedCounts[OldTag1]--;
				if (OldTag2.IsValid() && SimulatedCounts.Contains(OldTag2)) SimulatedCounts[OldTag2]--;
			}
		}
	}

	// [더하기] 새로 얻을 아이템의 시너지 증가
	if (NewItemToSwap->GetItemDA())
	{
		FGameplayTag NewTag1 = NewItemToSwap->GetItemDA()->SynergyTag1;
		FGameplayTag NewTag2 = NewItemToSwap->GetItemDA()->SynergyTag2;

		if (NewTag1.IsValid()) SimulatedCounts.FindOrAdd(NewTag1)++;
		if (NewTag2.IsValid()) SimulatedCounts.FindOrAdd(NewTag2)++;
	}

	struct FSynergySortData
	{
		FGameplayTag Tag;
		int32 OriginalCount;
		int32 NewCount;
		FSynergyDataTable* RowData;
	};
	
	TArray<FSynergySortData> SortArray;

	// 1. 유효한 데이터만 추려서 임시 배열에 담기
	for (const TPair<FGameplayTag, int32>& Pair : SimulatedCounts)
	{
		FGameplayTag Tag = Pair.Key;
		int32 NewCount = Pair.Value;
		int32 OriginalCount = InvComp->GetCurrentSynergyCounts().FindRef(Tag); 

		if (OriginalCount == 0 && NewCount == 0) continue;

		FSynergyDataTable* SynergyData = SynergyCache.FindRef(Tag);
		if (!SynergyData) continue;

		// 배열에 추가
		SortArray.Add({Tag, OriginalCount, NewCount, SynergyData});
	}

	// 2. ✨ NewCount가 높은 순서(내림차순)로 배열 정렬하기
	SortArray.Sort([](const FSynergySortData& A, const FSynergySortData& B) {
		return A.NewCount > B.NewCount;
	});

	// ==========================================
	// 3. 정렬된 배열을 바탕으로 위젯 생성 및 ScrollBox에 추가
	// ==========================================
	for (const FSynergySortData& Data : SortArray)
	{
		if (UCAP_SynergySimulSlotWidget* SimSlot = CreateWidget<UCAP_SynergySimulSlotWidget>(this, SynergySimulSlotClass))
		{
			UTexture2D* LoadedIcon = Data.RowData->SynergyIcon.LoadSynchronous();
			SimSlot->InitSlot(LoadedIcon, Data.OriginalCount, Data.NewCount);

			if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(TopSynergyScrollBox->AddChild(SimSlot)))
			{
				ScrollSlot->SetPadding(FMargin(10.f, 0.f, 10.f, 0.f)); 
				ScrollSlot->SetVerticalAlignment(VAlign_Center);
			}
		}
	}
}

