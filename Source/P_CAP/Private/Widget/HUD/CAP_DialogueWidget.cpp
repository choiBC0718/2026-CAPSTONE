// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_DialogueWidget.h"

#include "CAP_GameplayWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Interactables/NPC/CAP_WorldNPC.h"

void UCAP_DialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player)
		return;

	SetIsFocusable(true);

	SpecialBtn->OnClicked.AddDynamic(this, &UCAP_DialogueWidget::OnSpecialBtnClicked);
	TalkBtn->OnClicked.AddDynamic(this, &UCAP_DialogueWidget::OnTalkBtnClicked);
	QuitBtn->OnClicked.AddDynamic(this, &UCAP_DialogueWidget::OnQuitBtnClicked);
	
	ActiveTextBlocks.Add(Talk);
	ActiveTextBlocks.Add(SpecialActionText);
	ActiveTextBlocks.Add(Quit);
	
	ActiveButtons.Add(TalkBtn);
	ActiveButtons.Add(SpecialBtn);
	ActiveButtons.Add(QuitBtn);
	
	for (int32 i = 0; i < ActiveButtons.Num(); i++)
	{
		ActiveButtons[i]->OnHovered.AddDynamic(this, &UCAP_DialogueWidget::OnBtnHovered);
	}
}

void UCAP_DialogueWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);
	if (Animation == StartDialogueAnim && bIsClosing)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

FReply UCAP_DialogueWidget::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent)
{
	FKey Key = KeyEvent.GetKey();
	if (Key == EKeys::W || Key == EKeys::Up)
	{
		if (ActiveButtons.Num() > 1)
		{
			if (CurrentSelectedIndex==0)
				CurrentSelectedIndex = 2;
			else
				CurrentSelectedIndex--;
		}
		RefreshButtonVisuals();
		return FReply::Handled();
	}
	else if (Key == EKeys::S || Key == EKeys::Down)
	{
		if (ActiveButtons.Num() > 1)
		{
			if (CurrentSelectedIndex==2)
				CurrentSelectedIndex = 0;
			else
				CurrentSelectedIndex++;
		}
		RefreshButtonVisuals();
		return FReply::Handled();
	}
	else if (Key == EKeys::F || Key == EKeys::Enter)
	{
		if (ActiveButtons.IsValidIndex(CurrentSelectedIndex))
		{
			ActiveButtons[CurrentSelectedIndex]->OnClicked.Broadcast();
		}
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(Geometry, KeyEvent);
}

void UCAP_DialogueWidget::StartDialogue()
{
	bIsClosing = false;
	PlayAnimation(StartDialogueAnim);
}

void UCAP_DialogueWidget::UpdateDialogueUI(const FNPCData& Data)
{
	CachedNPCData = Data;
	bIsConfirming = false;

	bool bShowSpecial = !Data.SpecialActionText.IsEmpty();
	SetupActiveButtons(true, bShowSpecial, true);
	
	if (bShowSpecial)
		SpecialActionText->SetText(FText::FromString(Data.SpecialActionText));
	
	if (Data.NPCImage)
	{
		NPC_Image->SetBrushFromTexture(Data.NPCImage);
		NPC_Image->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		NPC_Image->SetVisibility(ESlateVisibility::Hidden);
	}
	
	NPC_Name->SetText(FText::FromString(Data.NPCName));
	DialogueText->SetText(FText::FromString(Data.DefaultDialogue));
}

void UCAP_DialogueWidget::ChangeToRewardState(const FString& ResultText)
{
	SetupActiveButtons(false,false,true);
	DialogueText->SetText(FText::FromString(ResultText));
	
	Quit->SetText(FText::FromString(TEXT("대화종료")));
}

void UCAP_DialogueWidget::OnSpecialBtnClicked()
{
	if (bIsClosing || !Player)
		return;
	
	if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
	{
		if (!bIsConfirming)
		{
			int32 RequiredCost = 0;
			if (InteractComp->GetNPCSpecialActionCost(RequiredCost))
			{
				if (const FString* ConfirmFormat = CachedNPCData.ResultDialogues.Find(ENPCActionResult::RequireConfirm))
				{	// 재화 얼마 소모되는지 보여주는 메세지로 교체
					FString FormatMsg = ConfirmFormat->Replace(TEXT("{Cost}"), *FString::FromInt(RequiredCost));
					// 이 안에서 bIsConfirming = true바꾸고, SpecialBtn 누르면 해당 메소드 재실행 (이부분은 스킵) 
					ChangeToConfirmState(FormatMsg);
					return;
				}
				else
				{	//설정 안한 경우 크래시 방지
					ChangeToConfirmState(FString::Printf(TEXT("마석 %d개가 필요하다. 지불하겠나?"), RequiredCost));
					return;
				}
			}
		}
		
		ENPCActionResult Result = InteractComp->ExecuteNPCSpecialAction();
		FString ResultText = TEXT("");
		if (const FString* FoundText = CachedNPCData.ResultDialogues.Find(Result))
		{
			ResultText = *FoundText;
		}
		else
			return;

		bIsConfirming = false;
		ChangeToRewardState(ResultText);
	}
	UE_LOG(LogTemp, Warning, TEXT("NPC 특수 로직 실행"));
}	

void UCAP_DialogueWidget::OnTalkBtnClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("NPC 떠들기 시작"));
}

void UCAP_DialogueWidget::OnQuitBtnClicked()
{
	if (bIsClosing)
		return;
	bIsClosing = true;
	PlayAnimation(StartDialogueAnim,0.f,1,EUMGSequencePlayMode::Reverse);
	OnDialogueFinished.Broadcast();
}

void UCAP_DialogueWidget::OnBtnHovered()
{
	for (int32 i = 0; i < ActiveButtons.Num(); i++)
	{
		if (ActiveButtons[i]->IsHovered())
		{
			CurrentSelectedIndex = i;
			RefreshButtonVisuals();
			break;
		}
	}
}

void UCAP_DialogueWidget::RefreshButtonVisuals()
{
	for (int32 i = 0; i < ActiveButtons.Num(); i++)
	{
		FButtonStyle Style = ActiveButtons[i]->GetStyle();
		Style.Normal.OutlineSettings.Width = 0.f;
		Style.Hovered.OutlineSettings.Width = 0.f;
		ActiveButtons[i]->SetStyle(Style);
		
		ActiveButtons[i]->SetBackgroundColor(ButtonNormalColor);

		FSlateFontInfo NormalFont = ActiveTextBlocks[i]->GetFont();
		NormalFont.Size=NormalFontSize;
		ActiveTextBlocks[i]->SetFont(NormalFont);

		if (CurrentSelectedIndex == i)
		{
			ActiveButtons[i]->SetBackgroundColor(ButtonHoverColor);
			
			Style.Normal.OutlineSettings.Color=ButtonHoverOutlineColor;
			Style.Normal.OutlineSettings.Width=ButtonHoverOutlineWidth;
			Style.Hovered.OutlineSettings.Color=ButtonHoverOutlineColor;
			Style.Hovered.OutlineSettings.Width=ButtonHoverOutlineWidth;
			ActiveButtons[i]->SetStyle(Style);

			FSlateFontInfo HoveredFont = ActiveTextBlocks[i]->GetFont();
			HoveredFont.Size=HoverFontSize;
			ActiveTextBlocks[i]->SetFont(HoveredFont);
		}
	}
}

void UCAP_DialogueWidget::ChangeToConfirmState(const FString& ConfirmText)
{
	bIsConfirming = true;
	DialogueText->SetText(FText::FromString(ConfirmText));

	SpecialActionText->SetText(FText::FromString(TEXT("수락")));
	Quit->SetText(FText::FromString(TEXT("거절")));

	SetupActiveButtons(false,true,true);
}

void UCAP_DialogueWidget::SetupActiveButtons(bool bShowTalk, bool bShowSpecial, bool bShowQuit)
{
	ActiveButtons.Empty();
	ActiveTextBlocks.Empty();

	auto ConfigureBtn = [&](UButton* InBtn, UTextBlock* InText, bool bVisible)
	{
		if (InBtn && InText)
		{
			if (bVisible)
			{
				InBtn->SetVisibility(ESlateVisibility::Visible);
				ActiveButtons.Add(InBtn);
				ActiveTextBlocks.Add(InText);
			}
			else
			{
				InBtn->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	};
	ConfigureBtn(TalkBtn, Talk, bShowTalk);
	ConfigureBtn(SpecialBtn, SpecialActionText, bShowSpecial);
	ConfigureBtn(QuitBtn, Quit, bShowQuit);

	CurrentSelectedIndex = 0;
	RefreshButtonVisuals();
}
