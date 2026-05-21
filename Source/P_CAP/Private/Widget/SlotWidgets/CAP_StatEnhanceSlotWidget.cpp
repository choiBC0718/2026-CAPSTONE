// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_StatEnhanceSlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_StatEnhanceTypes.h"

void UCAP_StatEnhanceSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

void UCAP_StatEnhanceSlotWidget::InitSlot(class ACAP_PlayerCharacter* Player)
{
	if (StatEnhanceDataTableRow.IsNull())
		return;

	FStatEnhanceTableRow* RowData = StatEnhanceDataTableRow.GetRow<FStatEnhanceTableRow>("");
	if (RowData)
	{
		if (StatIcon)
		{
			if (RowData->Icon.IsPending())
				StatIcon->SetBrushFromTexture(RowData->Icon.LoadSynchronous());
			else if (RowData->Icon.IsValid())
			{
				StatIcon->SetBrushFromTexture(RowData->Icon.Get());
			}
		}

		if (LevelText)
		{
			int32 CurrentLevel = 0;
			LevelText->SetText(FText::FromString(FString::Printf(TEXT("Lv. %d"),CurrentLevel)));
		}
	}
}

void UCAP_StatEnhanceSlotWidget::SetSlotSelected(bool bIsSelected)
{
	if (FocusBorderImg)
	{
		FocusBorderImg->SetVisibility(bIsSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

FReply UCAP_StatEnhanceSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().SetUserFocus(TakeWidget()).CaptureMouse(TakeWidget());
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCAP_StatEnhanceSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (HasMouseCapture())
	{
		// 소유했던 마우스 캡처를 풀어줍니다.
		FReply Reply = FReply::Handled().ReleaseMouseCapture();

		// 마우스를 뗄 때, 마우스 커서가 여전히 이 슬롯 안에 있는지 검사
		if (InGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				OnEnhanceSlotFocused.Broadcast(this);
			}
		}
		// 만약 슬롯 밖에서 마우스를 뗐다면 아무 일도 일어나지 않고 클릭 취소
		return Reply;
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}