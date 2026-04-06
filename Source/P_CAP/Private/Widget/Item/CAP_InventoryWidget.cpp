// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_InventoryWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/WrapBox.h"

void UCAP_InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCAP_InventoryWidget::RefreshLeftPanel(ACAP_PlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter || !SlotWidgetClass)
		return;

	WeaponBox->ClearChildren();
	ItemWrapBox->ClearChildren();

	for (int32 i=0; i<2;i++)
	{
		UCAP_ItemSlotWidget* WeaponSlot = CreateWidget<UCAP_ItemSlotWidget>(this,SlotWidgetClass);
		if (WeaponSlot)
		{
			//TODO 셀제 무기 아이콘 WeaponInstance 넣어줌
			// WeaponSlot -> InitSlot(WeaponIconTexture, WeaponInstance)

			WeaponSlot->OnSlotFocused.AddDynamic(this, &UCAP_InventoryWidget::OnAnySlotFocused);

			WeaponBox->AddChildToHorizontalBox(WeaponSlot);
			if (i==0)
				WeaponSlot->SetFocus();
		}
	}
}

void UCAP_InventoryWidget::HandleInteractInput(bool bIsPressed)
{
	if (!CurrentFocusedSlot)
		return;
	if (CurrentFocusedSlot->SlotType == ESlotItemType::Weapon)
	{
		if (bIsPressed)
		{
			// 무기 스킬 순서 변경
		}
	}
	else
	{
		if (bIsPressed)
		{
			//분해 시작
		}
		else
		{
			//분해 취소
		}
	}
}

void UCAP_InventoryWidget::OnAnySlotFocused(UCAP_ItemSlotWidget* FocusedSlot)
{
	if (!FocusedSlot)
		return;

	CurrentFocusedSlot = FocusedSlot;
	// 오른쪽 패널 업데이트
	//UpdateRightPanel(CurrentFocusedSlot->SlotItemData);
}
