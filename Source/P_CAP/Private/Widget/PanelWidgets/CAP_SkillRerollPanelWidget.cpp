// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_SkillRerollPanelWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Framework/Library/CAP_WidgetHelper.h"
#include "Interactables/NPC/NPC_SkillReroll.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"

void UCAP_SkillRerollPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CachedPlayer = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (CachedPlayer)
		OwnerNPC = Cast<ANPC_SkillReroll>(CachedPlayer->GetInteractionComponent()->GetNearbyInteractable());
	
	if (InnerEnhanceBtn)
	{
		InnerEnhanceBtn->OnClicked.AddDynamic(this, &UCAP_SkillRerollPanelWidget::OnRerollClicked);
		InnerEnhanceBtn->OnHovered.AddDynamic(this, &UCAP_SkillRerollPanelWidget::OnBtnHovered);
	}
	if (InnerCloseBtn)
	{
		InnerCloseBtn->OnClicked.AddDynamic(this, &UCAP_SkillRerollPanelWidget::OnCloseClicked);
		InnerCloseBtn->OnHovered.AddDynamic(this, &UCAP_SkillRerollPanelWidget::OnBtnHovered);
	}
	CurrentButtonIndex = 0;
	RefreshButtonVisuals();
}

void UCAP_SkillRerollPanelWidget::NativeOpenMenu()
{
	if (OpenAnim)
		PlayAnimation(OpenAnim);
	if (DialogueText && OwnerNPC)
		DialogueText->SetText(OwnerNPC->GetDialogueText(EEnhanceResult::Default));
}

void UCAP_SkillRerollPanelWidget::NativeCloseMenu()
{
	if (OpenAnim)
		PlayAnimation(OpenAnim, 0.f,1,EUMGSequencePlayMode::Reverse);
	OnMenuClosed.Broadcast();
}

void UCAP_SkillRerollPanelWidget::HandleChangeSelectedSlot(FVector2D InputVal)
{
	if (InputVal.Y > 0 || InputVal.Y < 0)
	{
		CurrentButtonIndex = CurrentButtonIndex == 0 ? 1 : 0;
	}
	RefreshButtonVisuals();
}

void UCAP_SkillRerollPanelWidget::HandleUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	if (TriggerEvent != ETriggerEvent::Started)
		return;

	if (CurrentButtonIndex == 1)
	{
		OnCloseClicked();
		return;
	}
	
	OnRerollClicked();
}

void UCAP_SkillRerollPanelWidget::OnRerollClicked()
{
	if (!OwnerNPC || !CachedPlayer) return;

	if (!bIsConfirmMode)
	{
		SetConfirmMode(true);
	}
	else
	{
		EEnhanceResult Result = OwnerNPC->TryRerollSkill(CachedPlayer);
		if (Result == EEnhanceResult::InsufficientCurrency || Result==EEnhanceResult::Success)
			SetConfirmMode(false);
		
		if (DialogueText)
			DialogueText->SetText( OwnerNPC->GetDialogueText(Result));
	}
}

void UCAP_SkillRerollPanelWidget::OnCloseClicked()
{
	if (bIsConfirmMode)
		SetConfirmMode(false);
	else
		NativeCloseMenu(); 
}

void UCAP_SkillRerollPanelWidget::OnBtnHovered()
{
	if (InnerEnhanceBtn && InnerEnhanceBtn->IsHovered())
		CurrentButtonIndex = 0;
	else if (InnerCloseBtn && InnerCloseBtn->IsHovered())
		CurrentButtonIndex = 1;
	
	RefreshButtonVisuals();
}

void UCAP_SkillRerollPanelWidget::SetConfirmMode(bool bIsConfirm)
{
	bIsConfirmMode = bIsConfirm;
	if (bIsConfirmMode)
	{	// 최종 강화 진행 확인
		if (InnerEnhanceText)
			InnerEnhanceText->SetText(FText::FromString(TEXT("변경")));
		if (InnerCloseText)
			InnerCloseText->SetText(FText::FromString(TEXT("거절")));
		
		int32 RequireCost = 0;
		if (UCAP_WeaponInstance* CurrentWeapon = CachedPlayer->GetWeaponComponent()->GetCurrentWeaponInstance())
		{
			EItemGrade CurrentWeaponGrade = CurrentWeapon->GetCurrentGrade();
			RequireCost = OwnerNPC->RerollCostMap[CurrentWeaponGrade];
		}
		
		FText DialogueToDisplay = OwnerNPC->GetDialogueText(EEnhanceResult::ConfirmMode, RequireCost);
		if (DialogueText)
			DialogueText->SetText(DialogueToDisplay);
	}
	else
	{	// 강화 직전에 떠들 때
		if (InnerEnhanceText)
			InnerEnhanceText->SetText(FText::FromString(TEXT("변경")));
		if (InnerCloseText)
			InnerCloseText->SetText(FText::FromString(TEXT("닫기")));
		if (DialogueText && OwnerNPC)
			DialogueText->SetText(OwnerNPC->GetDialogueText(EEnhanceResult::Default));
	}
}

void UCAP_SkillRerollPanelWidget::RefreshButtonVisuals()
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
