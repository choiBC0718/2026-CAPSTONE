// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_ItemEquipPanelWidget.h"

#include "CAP_ItemSlotWidget.h"
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
}

void UCAP_ItemEquipPanelWidget::RefreshPanel(ACAP_PlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter || !ItemSlotWidgetClass)
		return;

	// 장착된 무기 슬롯 갱신
	if (WeaponList)
	{
		WeaponList->ClearChildren();
		if (UCAP_WeaponComponent* WeaponComp = PlayerCharacter->GetWeaponComponent())
		{
			const TArray<UCAP_WeaponInstance*> EquippedWeapons = WeaponComp->GetEquippedWeapons();
			int MaxWeaponSlots = EquippedWeapons.Num();
			WeaponSlots.Empty();

			for (int i=0 ; i<MaxWeaponSlots ; i++)
			{
				UCAP_ItemSlotWidget* NewWeaponSlot = CreateWidget<UCAP_ItemSlotWidget>(GetOwningPlayer(), ItemSlotWidgetClass);
				if (NewWeaponSlot)
				{
					NewWeaponSlot->SetSlotNumber(i);

					UTexture2D* LoadedIcon = nullptr;
					UCAP_WeaponInstance* CurrentWeapon = EquippedWeapons[i];
					if (CurrentWeapon && CurrentWeapon->GetWeaponDA())
					{
						LoadedIcon = CurrentWeapon->GetWeaponDA()->WeaponIcon.LoadSynchronous();
					}
					NewWeaponSlot->InitSlot(ESlotItemType::Weapon, LoadedIcon, CurrentWeapon);

					UWrapBoxSlot* WrapSlot = WeaponList->AddChildToWrapBox(NewWeaponSlot);
					WrapSlot->SetPadding(FMargin(15.f));
					WeaponSlots.Add(NewWeaponSlot);

					NewWeaponSlot->OnLeftMouseClick.AddUObject(this, &UCAP_ItemEquipPanelWidget::HandleSlotLeftClicked);
				}
			}
		}
	}

	// 패시브 아이템 슬롯 갱신
	if (PassiveItemList)
	{
		InventoryComponent = PlayerCharacter->GetInventoryComponent();
		if (InventoryComponent)
		{
			int Capacity = InventoryComponent->GetCapacity();
			PassiveItemList->ClearChildren();
			ItemSlots.Empty();

			const TArray<UCAP_ItemInstance*> InventoryItems = InventoryComponent->GetInventoryItems();
			
			for (int i=0; i<Capacity; i++)
			{
				UCAP_ItemSlotWidget* NewItemSlot = CreateWidget<UCAP_ItemSlotWidget>(GetOwningPlayer(), ItemSlotWidgetClass);
				if (NewItemSlot)
				{
					NewItemSlot->SetSlotNumber(i);

					UTexture2D* LoadedIcon = nullptr;
					UCAP_ItemInstance* CurrentItem = nullptr;
					if (InventoryItems.IsValidIndex(i))
					{
						CurrentItem = InventoryItems[i];
						if (CurrentItem && CurrentItem->GetItemDA())
						{
							LoadedIcon = CurrentItem->GetItemDA()->ItemIcon.LoadSynchronous();
						}
					}
					// 아이템 아이콘 및 데이터 넘기기
					NewItemSlot->InitSlot(ESlotItemType::Item, LoadedIcon, CurrentItem);
					
					UWrapBoxSlot* WrapSlot = PassiveItemList->AddChildToWrapBox(NewItemSlot);
					WrapSlot->SetPadding(FMargin(15.f));
					ItemSlots.Add(NewItemSlot);

					NewItemSlot->OnRightMouseClick.AddUObject(this, &UCAP_ItemEquipPanelWidget::HandleSlotRightClicked);
					NewItemSlot->OnLeftMouseClick.AddUObject(this, &UCAP_ItemEquipPanelWidget::HandleSlotLeftClicked);
				}
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

void UCAP_ItemEquipPanelWidget::InitNearbySlot()
{
	int ItemColumns = 3; // 아이템 가로줄 개수

	// 1. 무기 슬롯 연결
	for (int i = 0; i < WeaponSlots.Num(); i++)
	{
		// 슬롯의 왼쪽 슬롯 지정
		if (i > 0) WeaponSlots[i]->LeftSlot = WeaponSlots[i - 1];
		else WeaponSlots[i]->LeftSlot = WeaponSlots.Last();

		// 슬롯의 오른쪽 슬롯 지정
		if (i < WeaponSlots.Num() - 1) WeaponSlots[i]->RightSlot = WeaponSlots[i + 1];
		else WeaponSlots[i]->RightSlot = ItemSlots.IsValidIndex(0) ? ItemSlots[0] : WeaponSlots[0];

		// 아래 슬롯 지정 (아이템 첫 줄 슬롯)
		if (ItemSlots.IsValidIndex(i)) WeaponSlots[i]->DownSlot = ItemSlots[i];

		// 위 슬롯 지정 (아이템 마지막 줄 슬롯)
		int BottomItemIdx = ItemSlots.Num() - ItemColumns + i; 
		if (ItemSlots.IsValidIndex(BottomItemIdx)) WeaponSlots[i]->UpSlot = ItemSlots[BottomItemIdx];
		else WeaponSlots[i]->UpSlot = ItemSlots.Num() > 0 ? ItemSlots.Last() : nullptr;
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
