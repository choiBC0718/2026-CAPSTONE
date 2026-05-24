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
	if (SlotWrapBox)
		SlotWrapBox->ClearChildren();
}

void UCAP_StatEnhancePanelWidget::NativeOpenMenu()
{
	if (!SlotWrapBox || !SlotWidgetClass || !EnhanceDataTable) return;

	ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player) return;
	
	TArray<FName> RowNames = EnhanceDataTable->GetRowNames();
	int32 RequiredSlots = FMath::Min(RowNames.Num(), 15);

	for (int32 i = 0; i < RequiredSlots; ++i)
	{
		UCAP_StatEnhanceSlotWidget* CurrentSlot = nullptr;
		if (CreatedSlots.IsValidIndex(i))
		{	// 이미 생성된 슬롯이라면 가져오기만
			CurrentSlot = CreatedSlots[i];
			CurrentSlot->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{	// 없었다면 슬롯 생성
			CurrentSlot = CreateWidget<UCAP_StatEnhanceSlotWidget>(this, SlotWidgetClass);
			if (CurrentSlot)
			{
				CurrentSlot->OnEnhanceSlotFocused.AddUObject(this, &UCAP_StatEnhancePanelWidget::HandleSlotFocused);
				SlotWrapBox->AddChild(CurrentSlot);
				CreatedSlots.Add(CurrentSlot);
			}
		}

		if (CurrentSlot)
		{
			CurrentSlot->StatEnhanceDataTableRow.DataTable=EnhanceDataTable;
			CurrentSlot->StatEnhanceDataTableRow.RowName=RowNames[i];
			CurrentSlot->InitSlot(Player);
		}
	}
	for (int32 i = RequiredSlots; i < CreatedSlots.Num(); ++i)
	{
		if (CreatedSlots[i])
			CreatedSlots[i]->SetVisibility(ESlateVisibility::Collapsed);
	}

	InitNearbySlot(RequiredSlots);
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
	{	// 컨펌모드가 아닐경우 -> 선택된 슬롯 강화할지 선택하도록 모드 변경
		if (CurrentSelectedSlot)
			SetConfirmMode(true);
	}
	else
	{	// 컨펌모드에서 선택은 버튼 2개 중 하나 (강화/취소)
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
	if (DetailWidget)
	{
		FStatEnhanceTableRow* RowData = nullptr;
		ACAP_PlayerCharacter* Player = nullptr;
		FName RowName;
		int32 CurrentLv = 0;
		if (TryGetEnhanceData(FocusedSlot, RowData, Player, RowName, CurrentLv))
		{
			FText FormattedDesc = GetFormattedDescription(RowData, CurrentLv);
			DetailWidget->UpdateDetailInfo(RowData->DisplayName, FormattedDesc, RowData->MaxLevel, CurrentLv);
		}
	}
}

void UCAP_StatEnhancePanelWidget::OnInnerEnhanceClicked()
{
	if (!bIsConfirmMode)
		SetConfirmMode(true);
	else
	{
		FStatEnhanceTableRow* RowData = nullptr;
		ACAP_PlayerCharacter* Player = nullptr;
		FName RowName;
		int32 CurrentLv = 0;
		
		if (TryGetEnhanceData(CurrentSelectedSlot, RowData, Player, RowName, CurrentLv))
		{	// 최대 레벨까지 찍지 않았으면 강화 진행
			if (Player->GetStatEnhanceComponent()->UpgradeStatEnhance(RowName, RowData->MaxLevel))
			{
				int32 RequiredCost = (CurrentLv + 1) * RowData->CostIncreaseRate;
				if (UCAP_CurrencyComponent* CurrencyComp = Player->GetCurrencyComponent())
				{	// 재화 충분한지 확인
					if (CurrencyComp->ConsumeCurrency(ECurrencyType::MagicStone, RequiredCost))
					{
						CurrentSelectedSlot->InitSlot(Player);
						HandleSlotFocused(CurrentSelectedSlot);
						SetConfirmMode(false);
						ApplyRandomDialogue(SuccessDialoguePool);
					}
					else
					{
						SetConfirmMode(false);
						ApplyRandomDialogue(FailDialoguePool);
					}
				}
			}
			else
			{	// 최대 레벨이 찍힌 상태라면 진행 취소 및 이미 만렙 대사
				SetConfirmMode(false);
				ApplyRandomDialogue(OnMaxLevelDialoguePool);
			}
		}
	}
}

void UCAP_StatEnhancePanelWidget::OnInnerCloseClicked()
{	// 컨펌모드일때 취소 : 강화 취소 / !컨펌모드일때 : 강화 위젯 제거
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

void UCAP_StatEnhancePanelWidget::InitNearbySlot(int32 ActiveSlotCount)
{
	if (ActiveSlotCount <=0)
		return;
	for (int32 i=0; i < ActiveSlotCount; ++i)
	{
		int32 Row = i/5;
		int32 Col = i%5;

		int32 RowStartIdx = Row * 5;
		int32 RowEndIdx = FMath::Min(RowStartIdx + 4, ActiveSlotCount - 1);
		
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
	HandleSlotFocused(CreatedSlots[0]);
}

void UCAP_StatEnhancePanelWidget::SetConfirmMode(bool bIsConfirm)
{
	bIsConfirmMode = bIsConfirm;
	if (bIsConfirmMode)
	{
		if (InnerCloseText) InnerCloseText->SetText(FText::FromString(TEXT("취소")));
		CurrentButtonIndex = 0;

		int32 RequiredCost = 0;
		FStatEnhanceTableRow* RowData = nullptr;
		ACAP_PlayerCharacter* Player = nullptr;
		FName RowName;
		int32 CurrentLv = 0;

		if (TryGetEnhanceData(CurrentSelectedSlot, RowData, Player, RowName, CurrentLv))
			RequiredCost = (CurrentLv + 1) * RowData->CostIncreaseRate;

		if (CurrentLv >= RowData->MaxLevel)
		{
			SetConfirmMode(false);
			ApplyRandomDialogue(OnMaxLevelDialoguePool);
			return;
		}
		ApplyRandomDialogue(ConfirmDialoguePool, RequiredCost);
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

bool UCAP_StatEnhancePanelWidget::TryGetEnhanceData(class UCAP_StatEnhanceSlotWidget* SlotWidget,
	struct FStatEnhanceTableRow*& OutRowData, class ACAP_PlayerCharacter*& OutPlayer, FName& OutRowName,
	int32& OutCurrentLevel)
{
	if (!SlotWidget || SlotWidget->StatEnhanceDataTableRow.IsNull())
		return false;
	OutPlayer = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!OutPlayer || !OutPlayer->GetStatEnhanceComponent())
		return false;
	OutRowData = SlotWidget->StatEnhanceDataTableRow.GetRow<FStatEnhanceTableRow>("");
	if (!OutRowData)
		return false;
	
	OutRowName = SlotWidget->StatEnhanceDataTableRow.RowName;
	OutCurrentLevel = OutPlayer->GetStatEnhanceComponent()->GetStatEnhanceLevel(OutRowName);
	return true;
}

FText UCAP_StatEnhancePanelWidget::GetFormattedDescription(const struct FStatEnhanceTableRow* RowData,int32 CurrentLevel)
{
	if (!RowData)
		return FText::GetEmpty();

	FFormatNamedArguments Args;
	for (int32 i = 0; i < RowData->Modifiers.Num(); ++i)
	{
		const FStatModifierInfo& ModInfo = RowData->Modifiers[i];
		float CalcVal = CurrentLevel == 0 ? 0.f : ModInfo.bIsFixedValue ? ModInfo.Value : static_cast<float>(CurrentLevel) * ModInfo.Value;
		
		if (ModInfo.bIsPercentage)
			CalcVal *= 100.f;

		FString ArgName = FString::Printf(TEXT("Value%d"), i);
		Args.Add(ArgName, FMath::RoundToInt(CalcVal));
	}
	return FText::Format(RowData->Description, Args);
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
