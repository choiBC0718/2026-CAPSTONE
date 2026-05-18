// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_DialogueWidget.h"

#include "CAP_GameplayWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "Interactables/NPC/CAP_WorldNPC.h"

void UCAP_DialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
	{
		InteractComp->OnDialogueTriggered.AddDynamic(this, &UCAP_DialogueWidget::OnNPCDialogueStarted);
	}

	SetIsFocusable(true);

	SpecialBtn->OnClicked.AddDynamic(this, &UCAP_DialogueWidget::OnSpecialBtnClicked);
	TalkBtn->OnClicked.AddDynamic(this, &UCAP_DialogueWidget::OnTalkBtnClicked);
	QuitBtn->OnClicked.AddDynamic(this, &UCAP_DialogueWidget::OnQuitBtnClicked);
	
	SetupActiveButtons(true,true,true);
	
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

		if (APlayerController* PC = GetOwningPlayer())
		{
			Player->EnableInput(PC);
			FInputModeGameAndUI InputMode;
			PC->SetInputMode(InputMode);
		}
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

FReply UCAP_DialogueWidget::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	SetKeyboardFocus();
	return FReply::Handled();
}

void UCAP_DialogueWidget::StartDialogue()
{
	bIsClosing = false;
	PlayAnimation(StartDialogueAnim);

	if (APlayerController* PC = GetOwningPlayer())
	{
		Player->DisableInput(PC);
		
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);

		SetKeyboardFocus();
	}
}

void UCAP_DialogueWidget::UpdateDialogueUI(const FNPCData& Data)
{
	CachedNPCData = Data;

	bool bShowSpecial = !Data.SpecialActionText.IsEmpty();
	SetupActiveButtons(true, bShowSpecial, true);

	if (Quit)
		Quit->SetText(FText::FromString(TEXT("대화종료")));
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

void UCAP_DialogueWidget::OnSpecialBtnClicked()
{
	if (!Player)
		return;
	
	UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent();
	if (!InteractComp)
		return;

	ENPCActionResult Result = InteractComp->ExecuteNPCSpecialAction();
	switch (Result)
	{
	case ENPCActionResult::OpenCustomWidget:
		OpenCustomWidget();
		break;

	case ENPCActionResult::FirstInteraction:
		HandleFirstInteraction(Result);
		break;

	case ENPCActionResult::RequireConfirm:
		HandleRequireConfirm(Result);
		break;

	case ENPCActionResult::Success:
		{
			FString ResultText=TEXT("NPC DialogueComponent에서 Success 시 대사 설정필요");
			if (const FString* FoundText = CachedNPCData.ResultDialogues.Find(Result))
				ResultText=*FoundText;
			ChangeToRewardState(ResultText);
		}
		break;
		
	case ENPCActionResult::InsufficientCurrency:
	case ENPCActionResult::AlreadyReceived:
	case ENPCActionResult::Failed:
		HandleFailedInteraction(Result);
		break;
	default:
		break;
	}
}	

void UCAP_DialogueWidget::OnTalkBtnClicked()
{
	DialogueText->SetText(FText::FromString(CachedNPCData.SmallTalkText));
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

// 대화가 시작되면 'Visible', 카메라 시점 전환
void UCAP_DialogueWidget::OnNPCDialogueStarted(const FNPCData& NPCData)
{
	SetVisibility(ESlateVisibility::Visible);
	StartDialogue();
	UpdateDialogueUI(NPCData);	// 상호작용 한 NPC의 설정에 맞게 텍스트, 이미지 설정

	if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
	{
		// 대화 종료 시점 구독후 기다림 (카메라 시점 전환 및 InputMode 변경)
		OnDialogueFinished.RemoveDynamic(InteractComp, &UCAP_InteractionComponent::EndDialogueCamera);
		OnDialogueFinished.AddDynamic(InteractComp, &UCAP_InteractionComponent::EndDialogueCamera);

		if (ACAP_WorldNPC* NPC = Cast<ACAP_WorldNPC>(InteractComp->GetNearbyInteractable()))
			NPC->ResetActionPending();
	}
}

void UCAP_DialogueWidget::OpenCustomWidget()
{
	ACAP_WorldNPC* TargetNPC = Cast<ACAP_WorldNPC>(Player->GetInteractionComponent()->GetNearbyInteractable());
	if (TargetNPC && TargetNPC->NPCSpecialActionWidgetClass)
	{
		if (ActiveCustomWidget)
			ActiveCustomWidget->RemoveFromParent();
		ActiveCustomWidget = CreateWidget<UUserWidget>(this, TargetNPC->NPCSpecialActionWidgetClass);
		
		if (CustomMenuWidget)
			CustomMenuWidget->AddChild(ActiveCustomWidget);

		SetupActiveButtons(false,false,true);
	}
}

void UCAP_DialogueWidget::HandleFirstInteraction(ENPCActionResult Result)
{
	FString ResultText = TEXT("NPC DialogueComponent에서 FirstInteraction 시 대사 설정필요");
	if (const FString* FoundText = CachedNPCData.ResultDialogues.Find(Result))
		ResultText=*FoundText;
	ChangeToConfirmState(ResultText);
}

void UCAP_DialogueWidget::HandleRequireConfirm(ENPCActionResult Result)
{
	int32 RequiredCost = 0;
	Player->GetInteractionComponent()->GetNPCSpecialActionCost(RequiredCost);

	FString ConfirmMsg= TEXT("NPC DialogueComponent에서 RequireConfirmed 시 대사 설정필요");
	if (const FString* ConfirmFormat = CachedNPCData.ResultDialogues.Find(Result))
		ConfirmMsg = ConfirmFormat->Replace(TEXT("{Cost}"), *FString::FromInt(RequiredCost));

	ChangeToConfirmState(ConfirmMsg);
}

void UCAP_DialogueWidget::HandleFailedInteraction(ENPCActionResult Result)
{
	FString ResultText = TEXT("NPC DialogueComponent에서 (재료 부족/기회 소진/Failed) 시 대사 설정필요");
	if (const FString* FoundText = CachedNPCData.ResultDialogues.Find(Result))
		ResultText = *FoundText;
	
	ChangeToRewardState(ResultText);
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
	DialogueText->SetText(FText::FromString(ConfirmText));

	SpecialActionText->SetText(FText::FromString(TEXT("수락")));
	Quit->SetText(FText::FromString(TEXT("거절")));

	SetupActiveButtons(false,true,true);
}

void UCAP_DialogueWidget::ChangeToRewardState(const FString& ResultText)
{
	SetupActiveButtons(false,false,true);
	DialogueText->SetText(FText::FromString(ResultText));
	
	Quit->SetText(FText::FromString(TEXT("대화종료")));
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
