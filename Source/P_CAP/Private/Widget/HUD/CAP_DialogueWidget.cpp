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
	
	ActiveTextBlocks.Add(SpecialActionText);
	ActiveTextBlocks.Add(Talk);
	ActiveTextBlocks.Add(Quit);
	
	ActiveButtons.Add(SpecialBtn);
	ActiveButtons.Add(TalkBtn);
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
	NPC_Image->SetBrushFromTexture(Data.NPCImage);
	NPC_Name->SetText(FText::FromString(Data.NPCName));
	DialogueText->SetText(FText::FromString(Data.DefaultDialogue));
	SpecialActionText->SetText(FText::FromString(Data.SpecialActionText));

	CurrentSelectedIndex =0;
	RefreshButtonVisuals();
}

void UCAP_DialogueWidget::ChangeToRewardState(const FString& ResultText)
{
	DialogueText->SetText(FText::FromString(ResultText));

	SpecialBtn->SetVisibility(ESlateVisibility::Hidden);
	TalkBtn->SetVisibility(ESlateVisibility::Hidden);

	ActiveButtons.Empty();
	ActiveTextBlocks.Empty();

	ActiveButtons.Add(QuitBtn);
	ActiveTextBlocks.Add(Quit);
	CurrentSelectedIndex =0;
	RefreshButtonVisuals();
}

void UCAP_DialogueWidget::OnSpecialBtnClicked()
{
	if (bIsClosing || !Player)
		return;

	ChangeToRewardState("gg");
	if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
	{
		InteractComp->ExecuteNPCSpecialAction();
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
