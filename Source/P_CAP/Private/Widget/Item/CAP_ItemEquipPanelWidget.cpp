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
					NewWeaponSlot->OnSlotFocused.AddDynamic(this, &UCAP_ItemEquipPanelWidget::HandleSlotFocused);

					UWrapBoxSlot* WrapSlot = WeaponList->AddChildToWrapBox(NewWeaponSlot);
					WrapSlot->SetPadding(FMargin(2.f));
					WeaponSlots.Add(NewWeaponSlot);
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
					NewItemSlot->OnSlotFocused.AddDynamic(this, &UCAP_ItemEquipPanelWidget::HandleSlotFocused);
					
					UWrapBoxSlot* WrapSlot = PassiveItemList->AddChildToWrapBox(NewItemSlot);
					WrapSlot->SetPadding(FMargin(2.f));
					ItemSlots.Add(NewItemSlot);
				}
			}
		}
	}
}

void UCAP_ItemEquipPanelWidget::HandleSlotFocused(UCAP_ItemSlotWidget* FocusedSlot)
{
	if (OnPanelSlotFocused.IsBound())
		OnPanelSlotFocused.Broadcast(FocusedSlot);
}
