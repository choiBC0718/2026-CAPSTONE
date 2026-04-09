// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_ItemSlotWidget.h"

#include "CAP_ItemEquipPanelWidget.h"
#include "Components/Image.h"

void UCAP_ItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	
}

void UCAP_ItemSlotWidget::InitSlot(ESlotItemType InSlotType, UTexture2D* InIcon, UObject* InItemData)
{
	SlotType = InSlotType;
	SlotItemData = InItemData;
	if (ItemIcon && InIcon)
	{
		ItemIcon->SetBrushFromTexture(InIcon);
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCAP_ItemSlotWidget::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber = NewSlotNumber;
}

void UCAP_ItemSlotWidget::SetSlotSelected(bool bIsSelected)
{
	if (FocusBorderImg)
	{
		FocusBorderImg->SetVisibility(bIsSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

FReply UCAP_ItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// 클릭하는 순간 이 슬롯에 포커스
		return FReply::Handled().SetUserFocus(TakeWidget()).CaptureMouse(TakeWidget());
	}
	return Super::NativeOnMouseButtonDown(Geometry, MouseEvent);
}

FReply UCAP_ItemSlotWidget::NativeOnMouseButtonUp(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	if (HasMouseCapture())
	{
		// 소유했던 마우스 캡처를 풀어줍니다.
		FReply Reply = FReply::Handled().ReleaseMouseCapture();

		// 마우스를 뗄 때, 마우스 커서가 여전히 이 슬롯 안에 있는지 검사
		if (Geometry.IsUnderLocation(MouseEvent.GetScreenSpacePosition()))
		{
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				LeftButtonClicked();
			}
			else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
			{
				RightButtonClicked();
			}
		}
		// 만약 슬롯 밖에서 마우스를 뗐다면 아무 일도 일어나지 않고 클릭 취소
		return Reply;
	}
	return Super::NativeOnMouseButtonUp(Geometry, MouseEvent);
}

void UCAP_ItemSlotWidget::RightButtonClicked()
{
	OnRightMouseClick.Broadcast(this);
}

void UCAP_ItemSlotWidget::LeftButtonClicked()
{
	OnLeftMouseClick.Broadcast(this);
}
