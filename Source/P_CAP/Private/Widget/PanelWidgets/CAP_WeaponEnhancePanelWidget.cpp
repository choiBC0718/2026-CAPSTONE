// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_WeaponEnhancePanelWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Interactables/NPC/NPC_WeaponEnhance.h"

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
	CurrentButtonIndex = -1;
	RefreshButtonVisuals();
}

void UCAP_WeaponEnhancePanelWidget::NativeOpenMenu()
{
	if (OpenAnim)
		PlayAnimation(OpenAnim);
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
	
	if (CurrentButtonIndex == 0)
		OnEnhanceClicked();
	else if (CurrentButtonIndex == 1)
		OnCloseClicked();
}

void UCAP_WeaponEnhancePanelWidget::OnEnhanceClicked()
{
	if (!OwnerNPC || !CachedPlayer) return;

	EWeaponUpgradeResult Result = OwnerNPC->TryUpgradeWeapon(CachedPlayer);
	FText DialogueToDisplay = OwnerNPC->GetDialogueText(Result);
	if (DialogueText)
		DialogueText->SetText(DialogueToDisplay);
}

void UCAP_WeaponEnhancePanelWidget::OnCloseClicked()
{
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

void UCAP_WeaponEnhancePanelWidget::RefreshButtonVisuals()
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

		if (CurrentButtonIndex == i)
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