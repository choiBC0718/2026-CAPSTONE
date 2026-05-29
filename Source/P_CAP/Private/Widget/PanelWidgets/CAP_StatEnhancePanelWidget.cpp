// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_StatEnhancePanelWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_StatEnhanceTypes.h"
#include "Framework/CAP_GameInstance.h"
#include "Interactables/NPC/NPC_StatEnhance.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/HUD/CAP_DialogueWidget.h"
#include "Widget/SlotWidgets/CAP_StatEnhanceDetailWidget.h"
#include "Widget/SlotWidgets/CAP_StatEnhanceSlotWidget.h"

void UCAP_StatEnhancePanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CachedPlayer = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (CachedPlayer)
		OwnerNPC = Cast<ANPC_StatEnhance>(CachedPlayer->GetInteractionComponent()->GetNearbyInteractable());
	
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
	if (!SlotWrapBox || !SlotWidgetClass) return;

	UDataTable* StatEnhanceDT = nullptr;
	if (UCAP_GameInstance* GI = Cast<UCAP_GameInstance>(UGameplayStatics::GetGameInstance(this)))
		StatEnhanceDT = GI->GetStatEnhanceTable();

	if (!StatEnhanceDT)
		return;

	ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player) return;
	
	TArray<FName> RowNames = StatEnhanceDT->GetRowNames();
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
			CurrentSlot->StatEnhanceDataTableRow.DataTable=StatEnhanceDT;
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
	if (DetailWidget && OwnerNPC)
	{
		FStatEnhanceTableRow* RowData = nullptr;
		FName RowName;
		int32 CurrentLv = 0;
		if (OwnerNPC->TryGetEnhanceData(FocusedSlot, CachedPlayer,RowData,RowName,CurrentLv))
		{
			FText FormattedDesc = OwnerNPC->GetFormattedDescription(RowData, CurrentLv);
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
		FName RowName;
		int32 CurrentLv = 0;
		
		if (OwnerNPC->TryGetEnhanceData(CurrentSelectedSlot, CachedPlayer,RowData,RowName,CurrentLv))
		{
			int32 RequiredCost = (CurrentLv+1) * RowData->CostIncreaseRate;
			EEnhanceResult Result = OwnerNPC->TryEnhanceStat(CachedPlayer, RowName,RequiredCost,RowData->MaxLevel);

			if (Result == EEnhanceResult::Success)
			{
				CurrentSelectedSlot->InitSlot(CachedPlayer);
				HandleSlotFocused(CurrentSelectedSlot);
				SetConfirmMode(false);
			}
			else if (Result == EEnhanceResult::InsufficientCurrency || Result == EEnhanceResult::MaxGradeReached)
				SetConfirmMode(false);

			if (DialogueText)
				DialogueText->SetText(OwnerNPC->GetDialogueText(Result));
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
		FName RowName;
		int32 CurrentLv = 0;

		if (OwnerNPC->TryGetEnhanceData(CurrentSelectedSlot, CachedPlayer,RowData,RowName,CurrentLv))
		{
			if (CurrentLv >= RowData->MaxLevel)
			{
				SetConfirmMode(false);
				if (DialogueText)
					DialogueText->SetText(OwnerNPC->GetDialogueText(EEnhanceResult::MaxGradeReached));
				return;
			}
			RequiredCost = (CurrentLv+1) * RowData->CostIncreaseRate;
		}
		if (DialogueText)
			DialogueText->SetText(OwnerNPC->GetDialogueText(EEnhanceResult::ConfirmMode, RequiredCost));
	}
	else
	{
		if (InnerCloseText)
			InnerCloseText->SetText(FText::FromString(TEXT("닫기")));
		if (DialogueText)
			DialogueText->SetText(OwnerNPC->GetDialogueText(EEnhanceResult::Default));
		CurrentButtonIndex = -1;
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
		
		bool bIsFocused = (CurrentButtonIndex == i); 
		UCAP_WidgetHelper::ApplyCustomButtonStyle(Buttons[i], Texts[i], bIsFocused, ButtonSettings);
	}
}
