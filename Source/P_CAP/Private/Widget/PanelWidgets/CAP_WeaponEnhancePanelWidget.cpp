// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_WeaponEnhancePanelWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Interactables/NPC/NPC_WeaponEnhance.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"

void UCAP_WeaponEnhancePanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CachedPlayer = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (CachedPlayer)
		OwnerNPC = Cast<ANPC_WeaponEnhance>(CachedPlayer->GetInteractionComponent()->GetNearbyInteractable());
	
	if (InnerEnhanceBtn)
	{
		InnerEnhanceBtn->OnClicked.AddDynamic(this, &UCAP_WeaponEnhancePanelWidget::OnEnhanceClicked);
		InnerEnhanceBtn->OnHovered.AddDynamic(this, &UCAP_WeaponEnhancePanelWidget::OnBtnHovered);
	}
	if (InnerCloseBtn)
	{
		InnerCloseBtn->OnClicked.AddDynamic(this, &UCAP_WeaponEnhancePanelWidget::OnCloseClicked);
		InnerCloseBtn->OnHovered.AddDynamic(this, &UCAP_WeaponEnhancePanelWidget::OnBtnHovered);
	}
	CurrentButtonIndex = 0;
	RefreshButtonVisuals();
}

void UCAP_WeaponEnhancePanelWidget::NativeOpenMenu()
{
	if (OpenAnim)
		PlayAnimation(OpenAnim);
	if (DialogueText && OwnerNPC)
		DialogueText->SetText(OwnerNPC->GetDialogueText(EEnhanceResult::Default));
}

void UCAP_WeaponEnhancePanelWidget::NativeCloseMenu()
{
	if (OpenAnim)
		PlayAnimation(OpenAnim, 0.f,1,EUMGSequencePlayMode::Reverse);
	OnMenuClosed.Broadcast();
}

void UCAP_WeaponEnhancePanelWidget::HandleChangeSelectedSlot(FVector2D InputVal)
{
	if (InputVal.Y > 0 || InputVal.Y < 0)
	{
		CurrentButtonIndex = CurrentButtonIndex == 0 ? 1 : 0;
	}
	RefreshButtonVisuals();
}

void UCAP_WeaponEnhancePanelWidget::HandleUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	if (TriggerEvent != ETriggerEvent::Started)
		return;

	if (CurrentButtonIndex == 1)
	{
		OnCloseClicked();
		return;
	}
	
	if (!bIsConfirmMode)
		SetConfirmMode(true);
	else
		OnEnhanceClicked();

}

void UCAP_WeaponEnhancePanelWidget::OnEnhanceClicked()
{
	if (!OwnerNPC || !CachedPlayer) return;

	if (!bIsConfirmMode)
		SetConfirmMode(true);
	else
	{
		EEnhanceResult Result = OwnerNPC->TryUpgradeWeapon(CachedPlayer);
		if (Result == EEnhanceResult::InsufficientCurrency || Result==EEnhanceResult::MaxGradeReached)
			SetConfirmMode(false);
		
		FText DialogueToDisplay = OwnerNPC->GetDialogueText(Result);
		if (DialogueText)
			DialogueText->SetText(DialogueToDisplay);
	}
}

void UCAP_WeaponEnhancePanelWidget::OnCloseClicked()
{
	if (bIsConfirmMode)
		SetConfirmMode(false);
	else
		NativeCloseMenu(); 
}

void UCAP_WeaponEnhancePanelWidget::OnBtnHovered()
{
	if (InnerEnhanceBtn && InnerEnhanceBtn->IsHovered())
		CurrentButtonIndex = 0;
	else if (InnerCloseBtn && InnerCloseBtn->IsHovered())
		CurrentButtonIndex = 1;
	
	RefreshButtonVisuals();
}

void UCAP_WeaponEnhancePanelWidget::SetConfirmMode(bool bIsConfirm)
{
	bIsConfirmMode = bIsConfirm;
	if (bIsConfirmMode)
	{	// 최종 강화 진행 확인
		if (InnerEnhanceText)
			InnerEnhanceText->SetText(FText::FromString(TEXT("지불")));
		if (InnerCloseText)
			InnerCloseText->SetText(FText::FromString(TEXT("거절")));
		
		int32 RequireCost = 0;
		if (UCAP_WeaponInstance* CurrentWeapon = CachedPlayer->GetWeaponComponent()->GetCurrentWeaponInstance())
		{
			EItemGrade CurrentWeaponGrade = CurrentWeapon->GetCurrentGrade();
			RequireCost = OwnerNPC->UpgradeCostMap[CurrentWeaponGrade];
		}
		
		FText DialogueToDisplay = OwnerNPC->GetDialogueText(EEnhanceResult::ConfirmMode, RequireCost);
		if (DialogueText)
			DialogueText->SetText(DialogueToDisplay);
	}
	else
	{	// 강화 직전에 떠들 때
		if (InnerEnhanceText)
			InnerEnhanceText->SetText(FText::FromString(TEXT("각성")));
		if (InnerCloseText)
			InnerCloseText->SetText(FText::FromString(TEXT("닫기")));
		if (DialogueText && OwnerNPC)
			DialogueText->SetText(OwnerNPC->GetDialogueText(EEnhanceResult::Default));
	}
}

void UCAP_WeaponEnhancePanelWidget::RefreshButtonVisuals()
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