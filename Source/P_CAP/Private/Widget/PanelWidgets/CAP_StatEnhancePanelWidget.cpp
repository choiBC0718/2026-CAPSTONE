// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_StatEnhancePanelWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_StatEnhanceTypes.h"
#include "Widget/HUD/CAP_DialogueWidget.h"
#include "Widget/SlotWidgets/CAP_StatEnhanceDetailWidget.h"
#include "Widget/SlotWidgets/CAP_StatEnhanceSlotWidget.h"

void UCAP_StatEnhancePanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	if (InnerEnhanceBtn)
	{
		InnerEnhanceBtn->OnClicked.AddDynamic(this, &UCAP_StatEnhancePanelWidget::OnInnerEnhanceClicked);
		InnerEnhanceBtn->OnHovered.AddDynamic(this, &UCAP_StatEnhancePanelWidget::OnButtonHovered);
	}
	if (InnerCloseBtn)
	{
		InnerCloseBtn->OnClicked.AddDynamic(this, &UCAP_StatEnhancePanelWidget::OnInnerCloseClicked);
		InnerCloseBtn->OnHovered.AddDynamic(this, &UCAP_StatEnhancePanelWidget::OnButtonHovered);
	}
	CurrentButtonIndex = -1;
	RefreshButtonVisuals();
}

void UCAP_StatEnhancePanelWidget::NativeOpenMenu()
{
	if (!SlotWrapBox || !SlotWidgetClass || !EnhanceDataTable) return;

	ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player) return;
	
	SlotWrapBox->ClearChildren();
	CreatedSlots.Empty();

	TArray<FName> RowNames = EnhanceDataTable->GetRowNames();
	int32 MaxSlots = FMath::Min(RowNames.Num(), 15);

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		UCAP_StatEnhanceSlotWidget* NewSlot = CreateWidget<UCAP_StatEnhanceSlotWidget>(this, SlotWidgetClass);
		if (!NewSlot) continue;

		NewSlot->StatEnhanceDataTableRow.DataTable = EnhanceDataTable;
		NewSlot->StatEnhanceDataTableRow.RowName = RowNames[i];
		NewSlot->InitSlot(Player);
		NewSlot->OnEnhanceSlotFocused.AddUObject(this, &UCAP_StatEnhancePanelWidget::HandleSlotFocused);

		SlotWrapBox->AddChildToWrapBox(NewSlot);
		CreatedSlots.Add(NewSlot);
	}

	InitNearbySlot();
	if (OpenAnim) 
		PlayAnimation(OpenAnim);
}

void UCAP_StatEnhancePanelWidget::NativeCloseMenu()
{
	if (OpenAnim)
		PlayAnimation(OpenAnim, 0.f,1,EUMGSequencePlayMode::Reverse);
	OnMenuClosed.Broadcast();
}

void UCAP_StatEnhancePanelWidget::HandleChangeSelectedSlot(FVector2D InputVal)
{
	if (!bIsConfirmMode)
	{
		if (!CurrentSelectedSlot)
			return;
		UCAP_StatEnhanceSlotWidget* NextSlot = nullptr;
	
		if (InputVal.X > 0)			NextSlot = CurrentSelectedSlot->RightSlot;
		else if (InputVal.X < 0)	NextSlot = CurrentSelectedSlot->LeftSlot;
		else if (InputVal.Y > 0)	NextSlot = CurrentSelectedSlot->UpSlot;
		else if (InputVal.Y < 0)	NextSlot = CurrentSelectedSlot->DownSlot;

		if (NextSlot)
			HandleSlotFocused(NextSlot);
	}
	else
	{
		if (InputVal.Y > 0 || InputVal.Y < 0)
		{
			CurrentButtonIndex = CurrentButtonIndex == 0 ? 1 : 0;
		}
		RefreshButtonVisuals();
	}
}

void UCAP_StatEnhancePanelWidget::HandleUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	if (TriggerEvent != ETriggerEvent::Started)
		return;
	
	if (!bIsConfirmMode)
	{
		if (CurrentSelectedSlot)
			SetConfirmMode(true);
	}
	else
	{
		if (CurrentButtonIndex == 0)
			OnInnerEnhanceClicked();
		else if (CurrentButtonIndex == 1)
			OnInnerCloseClicked();
	}
}

void UCAP_StatEnhancePanelWidget::HandleSlotFocused(UCAP_StatEnhanceSlotWidget* FocusedSlot)
{
	if (CurrentSelectedSlot && FocusedSlot!=CurrentSelectedSlot)
		CurrentSelectedSlot->SetSlotSelected(false);

	if (FocusedSlot)
	{
		CurrentSelectedSlot = FocusedSlot;
		CurrentSelectedSlot->SetSlotSelected(true);
	}
	if (DetailWidget && !FocusedSlot->StatEnhanceDataTableRow.IsNull())
	{
		FStatEnhanceTableRow* RowData = FocusedSlot->StatEnhanceDataTableRow.GetRow<FStatEnhanceTableRow>("");
		if (RowData)
		{
			int32 CurrentLv= 0;
			DetailWidget->UpdateDetailInfo(RowData->DisplayName, RowData->Description, RowData->MaxLevel, CurrentLv);
		}
	}
}

void UCAP_StatEnhancePanelWidget::OnInnerEnhanceClicked()
{
	if (!bIsConfirmMode)
	{
		SetConfirmMode(true);
	}
	else
	{
		// TODO: 실제 재화 소모 및 스탯 강화 비즈니스 로직 실행
		SetConfirmMode(false);
		ApplyRandomDialogue(SuccessDialoguePool);
	}
}

void UCAP_StatEnhancePanelWidget::OnInnerCloseClicked()
{
	if (bIsConfirmMode)
		SetConfirmMode(false);
	else
		NativeCloseMenu(); 
}

void UCAP_StatEnhancePanelWidget::OnButtonHovered()
{
	if (InnerEnhanceBtn && InnerEnhanceBtn->IsHovered())
		CurrentButtonIndex = 0;
	else if (InnerCloseBtn && InnerCloseBtn->IsHovered())
		CurrentButtonIndex = 1;
	
	RefreshButtonVisuals();
}

void UCAP_StatEnhancePanelWidget::InitNearbySlot()
{
	for (int32 i=0; i < CreatedSlots.Num(); ++i)
	{
		int32 Row = i/5;
		int32 Col = i%5;

		int32 RowStartIdx = Row * 5;
		int32 RowEndIdx = FMath::Min(RowStartIdx + 4, CreatedSlots.Num() - 1);
		
		// 왼쪽 슬롯
		CreatedSlots[i]->LeftSlot = Col>0 ? CreatedSlots[i - 1] : CreatedSlots[RowEndIdx];

		// 오른쪽 슬롯
		CreatedSlots[i]->RightSlot = i<RowEndIdx ? CreatedSlots[i + 1] : CreatedSlots[RowStartIdx];
		
		// 윗쪽 슬롯
		CreatedSlots[i]->UpSlot = Row > 0 ? CreatedSlots[i - 5] : 
		(CreatedSlots.IsValidIndex(i + 10) ? CreatedSlots[i + 10] : 
		(CreatedSlots.IsValidIndex(i + 5) ? CreatedSlots[i + 5] : CreatedSlots[i]));
    
		// 아래쪽 슬롯
		CreatedSlots[i]->DownSlot = CreatedSlots.IsValidIndex(i + 5) ? CreatedSlots[i + 5] : CreatedSlots[Col];
	}
	if (CreatedSlots.Num() > 0)
		HandleSlotFocused(CreatedSlots[0]);
}

void UCAP_StatEnhancePanelWidget::SetConfirmMode(bool bIsConfirm)
{
	bIsConfirmMode = bIsConfirm;
	if (bIsConfirmMode)
	{
		if (InnerCloseText) InnerCloseText->SetText(FText::FromString(TEXT("취소")));
		CurrentButtonIndex = 0;
		// 비용 가져오기
		ApplyRandomDialogue(ConfirmDialoguePool, 150);
	}
	else
	{
		if (InnerCloseText) InnerCloseText->SetText(FText::FromString(TEXT("닫기")));
		CurrentButtonIndex = -1;
		ApplyRandomDialogue(BrowseDialoguePool);
	}
	RefreshButtonVisuals();
}

void UCAP_StatEnhancePanelWidget::RefreshButtonVisuals()
{
	UButton* Buttons[2] = { InnerEnhanceBtn, InnerCloseBtn };
	UTextBlock* Texts[2] = { InnerEnhanceText, InnerCloseText };

	for (int32 i = 0; i < 2; ++i)
	{
		if (!Buttons[i] || !Texts[i]) continue;
		
		FButtonStyle Style = Buttons[i]->GetStyle();
		Style.Normal.OutlineSettings.Width = 0.f;
		Style.Hovered.OutlineSettings.Width = 0.f;
		Buttons[i]->SetStyle(Style);
		
		Buttons[i]->SetBackgroundColor(ButtonNormalColor);

		FSlateFontInfo FontInfo = Texts[i]->GetFont();
		FontInfo.Size = NormalFontSize;
		Texts[i]->SetFont(FontInfo);

		if (bIsConfirmMode && CurrentButtonIndex == i)
		{
			Buttons[i]->SetBackgroundColor(ButtonHoverColor);
			
			Style.Normal.OutlineSettings.Color = ButtonHoverOutlineColor;
			Style.Normal.OutlineSettings.Width = ButtonHoverOutlineWidth;
			Style.Hovered.OutlineSettings.Color = ButtonHoverOutlineColor;
			Style.Hovered.OutlineSettings.Width = ButtonHoverOutlineWidth;
			Buttons[i]->SetStyle(Style);

			FontInfo.Size = HoverFontSize;
			Texts[i]->SetFont(FontInfo);
		}
	}
}

void UCAP_StatEnhancePanelWidget::ApplyRandomDialogue(const TArray<FText>& DialoguePool, int32 Cost)
{
	if (DialoguePool.IsEmpty() || !DialogueText)
		return;

	int32 RandIdx = FMath::RandRange(0, DialoguePool.Num() - 1);
	FText SelectedDialogue = DialoguePool[RandIdx];
	if (Cost>=0)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Cost"),Cost);
		// {Cost} 로 입력한 자리에 숫자로 들어감
		DialogueText->SetText(FText::Format(SelectedDialogue, Args));
	}
	else
	{
		DialogueText->SetText(SelectedDialogue);
	}
}
