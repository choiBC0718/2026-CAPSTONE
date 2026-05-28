// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/Widget/CombatRewardChoiceCardWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCombatRewardChoiceCardWidget::InitializeCard(const FCombatRewardChoiceOption& InOption)
{
	Option = InOption;
	RefreshCard();
	OnCardInitialized(Option);
}

void UCombatRewardChoiceCardWidget::SelectCard()
{
	OnCardSelected.Broadcast(Option.RewardType);
}

void UCombatRewardChoiceCardWidget::RefreshCard()
{
	if (Text_Title)
	{
		Text_Title->SetText(Option.Title);
	}

	if (Text_Description)
	{
		Text_Description->SetText(Option.Description);
	}

	if (Text_Badge)
	{
		Text_Badge->SetText(Option.BadgeText);
	}

	if (Text_Cost)
	{
		Text_Cost->SetText(FText::AsNumber(Option.Cost));
	}

	if (Image_Artwork && Option.Image)
	{
		Image_Artwork->SetBrushFromTexture(Option.Image);
	}
}
