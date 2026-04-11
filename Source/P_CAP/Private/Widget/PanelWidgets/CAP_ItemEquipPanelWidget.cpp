// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_ItemEquipPanelWidget.h"

#include "Widget/SlotWidgets/CAP_ItemSlotWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Items/Item/CAP_InventoryComponent.h"
#include "Items/Item/CAP_ItemInstance.h"
#include "Items/Weapon/CAP_WeaponComponent.h"
#include "Items/Weapon/CAP_WeaponInstance.h"

void UCAP_ItemEquipPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (WeaponList)
	{
		WeaponList->ClearChildren();
		WeaponSlots.Empty();
	}
	if (PassiveItemList)
	{
		PassiveItemList->ClearChildren();
		ItemSlots.Empty();
	}
}

void UCAP_ItemEquipPanelWidget::RefreshPanel(ACAP_PlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter || !ItemSlotWidgetClass)
		return;

	// 장착된 무기 슬롯 갱신
	if (WeaponList)
	{
		if (UCAP_WeaponComponent* WeaponComp = PlayerCharacter->GetWeaponComponent())
		{
			const TArray<UCAP_WeaponInstance*> EquippedWeapons = WeaponComp->GetEquippedWeapons();
			for (int i=0 ; i<EquippedWeapons.Num() ; i++)
			{
				UCAP_WeaponInstance* WeaponInst = EquippedWeapons[i];
				UTexture2D* Icon = (WeaponInst && WeaponInst->GetWeaponDA()) ? WeaponInst->GetWeaponDA()->WeaponIcon.LoadSynchronous() : nullptr;

				if (WeaponSlots.IsValidIndex(i) && WeaponSlots[i] != nullptr)
				{
					WeaponSlots[i] -> InitSlot(ESlotItemType::Weapon, Icon, WeaponInst);
				}
				else
					CreateAndAddSlot(WeaponList, WeaponSlots, ESlotItemType::Weapon, i, WeaponInst, Icon);
			}
		}
	}

	// 패시브 아이템 슬롯 갱신
	if (PassiveItemList)
	{
		if (UCAP_InventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent())
		{
			const TArray<UCAP_ItemInstance*>& InventoryItems = InventoryComponent->GetInventoryItems();
			
			for (int i=0; i<InventoryComponent->GetCapacity(); i++)
			{
				UCAP_ItemInstance* ItemInst = InventoryItems.IsValidIndex(i) ? InventoryItems[i] : nullptr;
				UTexture2D* Icon = (ItemInst && ItemInst->GetItemDA()) ? ItemInst->GetItemDA()->ItemIcon.LoadSynchronous() : nullptr;

				// 배열에 위젯이 있다면 데이터 갈아끼우기
				if (ItemSlots.IsValidIndex(i) && ItemSlots[i] != nullptr)
				{
					ItemSlots[i] -> InitSlot(ESlotItemType::Item, Icon, ItemInst);
				}
				// 없었다면 새로 생성
				else
					CreateAndAddSlot(PassiveItemList, ItemSlots, ESlotItemType::Item, i, ItemInst, Icon);
			}
		}
	}
	InitNearbySlot();
}

void UCAP_ItemEquipPanelWidget::MoveSelection(FVector2D InputVal)
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

void UCAP_ItemEquipPanelWidget::HandleSlotLeftClicked(class UCAP_ItemSlotWidget* ClickedSlot)
{
	// 기존에 포커스 된 슬롯이 있다면 그 슬롯 테두리 제거
	if (CurrentSelectedSlot && CurrentSelectedSlot != ClickedSlot)
	{
		CurrentSelectedSlot->SetSlotSelected(false);
	}
	// 새로 클릭된 슬롯 테두리 활성화 후 현재 선택된 슬롯으로 등록
	if (ClickedSlot)
	{
		ClickedSlot->SetSlotSelected(true);
		CurrentSelectedSlot = ClickedSlot;
	}
	if (OnPanelSlotClicked.IsBound())
		OnPanelSlotClicked.Broadcast(ClickedSlot);
}

void UCAP_ItemEquipPanelWidget::HandleSlotRightClicked(class UCAP_ItemSlotWidget* ClickedSlot)
{
	if (OnPanelSlotClicked.IsBound())
		OnPanelSlotClicked.Broadcast(ClickedSlot);
}

void UCAP_ItemEquipPanelWidget::CreateAndAddSlot(UWrapBox* TargetBox, TArray<UCAP_ItemSlotWidget*>& TargetArray,
	ESlotItemType SlotType, int32 Index, UObject* ItemData, UTexture2D* Icon)
{
	if (UCAP_ItemSlotWidget* NewSlot = CreateWidget<UCAP_ItemSlotWidget>(GetOwningPlayer(), ItemSlotWidgetClass))
	{
		NewSlot->SetSlotNumber(Index);
		NewSlot->InitSlot(SlotType, Icon, ItemData);
		
		if (UWrapBoxSlot* WrapSlot = TargetBox->AddChildToWrapBox(NewSlot))
		{
			WrapSlot->SetPadding(FMargin(15.f));
		}
		
		NewSlot->OnLeftMouseClick.AddUObject(this, &UCAP_ItemEquipPanelWidget::HandleSlotLeftClicked);
		if (SlotType == ESlotItemType::Item)
		{
			NewSlot->OnRightMouseClick.AddUObject(this, &UCAP_ItemEquipPanelWidget::HandleSlotRightClicked);
		}
		TargetArray.Add(NewSlot);
	}
}

void UCAP_ItemEquipPanelWidget::InitNearbySlot()
{
	int ItemColumns = 3; // 아이템 가로줄 개수

	// 1. 무기 슬롯 연결
	for (int i = 0; i < WeaponSlots.Num(); i++)
	{
		// 슬롯의 왼쪽 슬롯 지정
		WeaponSlots[i]->LeftSlot = (i>0) ? WeaponSlots[i-1] : WeaponSlots.Last();

		// 슬롯의 오른쪽 슬롯 지정
		WeaponSlots[i]->RightSlot = (i<WeaponSlots.Num()-1) ? WeaponSlots[i+1] : (ItemSlots.IsValidIndex(0)) ? ItemSlots[0] : WeaponSlots[0];
		
		// 아래 슬롯 지정 (아이템 첫 줄 슬롯)
		WeaponSlots[i]->DownSlot = (ItemSlots.IsValidIndex(i)) ? ItemSlots[i] : nullptr;

		// 위 슬롯 지정 (아이템 마지막 줄 슬롯)
		int BottomItemIdx = ItemSlots.Num() - ItemColumns + i; 
		WeaponSlots[i]->UpSlot = ItemSlots.IsValidIndex(BottomItemIdx) ? ItemSlots[BottomItemIdx] : ItemSlots.Num() > 0 ? ItemSlots.Last() : nullptr;
	}

	// 2. 아이템 슬롯 연결
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
		else // 맨 윗줄이라면 무기 슬롯으로
			ItemSlots[i]->UpSlot = WeaponSlots.IsValidIndex(Col) ? WeaponSlots[Col] : (WeaponSlots.Num() > 0 ? WeaponSlots.Last() : nullptr);

		// 아래 슬롯
		if (Row < (ItemSlots.Num() - 1) / ItemColumns) 
			ItemSlots[i]->DownSlot = ItemSlots[i + ItemColumns];
		else // 맨 아랫줄이라면 무기 슬롯으로 다시 순환!
			ItemSlots[i]->DownSlot = WeaponSlots.IsValidIndex(Col) ? WeaponSlots[Col] : (WeaponSlots.Num() > 0 ? WeaponSlots.Last() : nullptr);
	}
	
	// 첫 번째 무기 강제 클릭 처리
	if (WeaponSlots.Num() > 0) HandleSlotLeftClicked(WeaponSlots[0]);
	else CurrentSelectedSlot = nullptr;
}
