// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_StatEnhanceSlotWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_StatEnhanceComponent.h"
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
			if (UCAP_StatEnhanceComponent* EnhanceComp = Player->GetStatEnhanceComponent())
			{
				FName RowName = StatEnhanceDataTableRow.RowName;
				CurrentLevel= EnhanceComp->GetStatEnhanceLevel(RowName);
			}
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

void UCAP_StatEnhanceSlotWidget::SetConfirmColor(FLinearColor Color)
{
	if (FocusBorderImg)
	{
		if (UMaterialInstanceDynamic* MID = FocusBorderImg->GetDynamicMaterial())
			MID->SetVectorParameterValue("HighlightColor",Color);
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
		FReply Reply = FReply::Handled().ReleaseMouseCapture();
		if (InGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		{
			if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				OnEnhanceSlotFocused.Broadcast(this);
			}
		}
		return Reply;
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}