// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/Widget/CombatRewardChoiceCardWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

void UCombatRewardChoiceCardWidget::InitializeCard(const FCombatRewardChoiceOption& InOption)
{
	Option = InOption;
	RefreshCard();
	OnCardInitialized(Option);
}

void UCombatRewardChoiceCardWidget::SelectCard()
{
	if (Option.RewardType == ECombatRoomRewardType::None)
	{
		return;
	}

	OnCardSelected.Broadcast(Option.RewardType);
}

void UCombatRewardChoiceCardWidget::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	ApplyVisualState(true);
}

void UCombatRewardChoiceCardWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	ApplyVisualState(false);
}

FReply UCombatRewardChoiceCardWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SelectCard();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UCombatRewardChoiceCardWidget::RefreshCard()
{
	const FLinearColor AccentColor = GetAccentColor();

	if (Text_Title)
	{
		Text_Title->SetText(Option.Title.IsEmpty() ? GetDefaultTitle() : Option.Title);
		Text_Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.94f, 0.86f, 1.0f)));
	}

	if (Text_Description)
	{
		Text_Description->SetText(Option.Description.IsEmpty() ? GetDefaultDescription() : Option.Description);
		Text_Description->SetAutoWrapText(true);
		Text_Description->SetWrapTextAt(170.0f);
		Text_Description->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 0.78f, 0.70f, 1.0f)));
	}

	if (Text_Badge)
	{
		const FText BadgeText = Option.BadgeText.IsEmpty() ? GetDefaultBadgeText() : Option.BadgeText;
		Text_Badge->SetText(BadgeText);
		Text_Badge->SetColorAndOpacity(FSlateColor(AccentColor));
		Text_Badge->SetVisibility(BadgeText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	if (Text_Cost)
	{
		Text_Cost->SetText(FText::AsNumber(Option.Cost));
		Text_Cost->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.82f, 0.36f, 1.0f)));
		Text_Cost->SetVisibility(Option.Cost > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Text_TypeGlyph)
	{
		Text_TypeGlyph->SetText(GetTypeGlyph());
		Text_TypeGlyph->SetColorAndOpacity(FSlateColor(AccentColor));
	}

	if (Image_Artwork && Option.Image)
	{
		Image_Artwork->SetBrushFromTexture(Option.Image);
		Image_Artwork->SetColorAndOpacity(FLinearColor::White);
	}

	if (Image_Artwork && !Option.Image)
	{
		Image_Artwork->SetColorAndOpacity(FLinearColor(AccentColor.R, AccentColor.G, AccentColor.B, 0.18f));
	}

	if (Border_Accent)
	{
		Border_Accent->SetBrushColor(AccentColor);
	}

	if (Border_ArtworkFrame)
	{
		Border_ArtworkFrame->SetBrushColor(FLinearColor(AccentColor.R, AccentColor.G, AccentColor.B, 0.24f));
	}

	ApplyVisualState(false);
}

void UCombatRewardChoiceCardWidget::ApplyVisualState(bool bHovered)
{
	if (Border_CardFrame)
	{
		Border_CardFrame->SetBrushColor(bHovered ? CardHoveredColor : CardNormalColor);
	}

	const float TargetScale = bHovered ? HoverScale : 1.0f;
	SetRenderScale(FVector2D(TargetScale, TargetScale));
}

FLinearColor UCombatRewardChoiceCardWidget::GetAccentColor() const
{
	switch (Option.RewardType)
	{
	case ECombatRoomRewardType::Gold:
		return GoldAccentColor;

	case ECombatRoomRewardType::Item:
		return ItemAccentColor;

	case ECombatRoomRewardType::Weapon:
		return WeaponAccentColor;

	default:
		return DefaultAccentColor;
	}
}

FText UCombatRewardChoiceCardWidget::GetDefaultTitle() const
{
	switch (Option.RewardType)
	{
	case ECombatRoomRewardType::Gold:
		return FText::FromString(TEXT("Gold"));

	case ECombatRoomRewardType::Item:
		return FText::FromString(TEXT("Item"));

	case ECombatRoomRewardType::Weapon:
		return FText::FromString(TEXT("Weapon"));

	default:
		return FText::FromString(TEXT("Reward"));
	}
}

FText UCombatRewardChoiceCardWidget::GetDefaultDescription() const
{
	switch (Option.RewardType)
	{
	case ECombatRoomRewardType::Gold:
		return FText::FromString(TEXT("방 클리어 시 골드를 획득합니다."));

	case ECombatRoomRewardType::Item:
		return FText::FromString(TEXT("방 클리어 시 아이템을 획득합니다."));

	case ECombatRoomRewardType::Weapon:
		return FText::FromString(TEXT("방 클리어 시 무기를 획득합니다."));

	default:
		return FText::FromString(TEXT("방 클리어 보상을 선택합니다."));
	}
}

FText UCombatRewardChoiceCardWidget::GetDefaultBadgeText() const
{
	switch (Option.RewardType)
	{
	case ECombatRoomRewardType::Gold:
		return FText::FromString(TEXT("Currency"));

	case ECombatRoomRewardType::Item:
		return FText::FromString(TEXT("Artifact"));

	case ECombatRoomRewardType::Weapon:
		return FText::FromString(TEXT("Gear"));

	default:
		return FText::FromString(TEXT("Reward"));
	}
}

FText UCombatRewardChoiceCardWidget::GetTypeGlyph() const
{
	switch (Option.RewardType)
	{
	case ECombatRoomRewardType::Gold:
		return FText::FromString(TEXT("G"));

	case ECombatRoomRewardType::Item:
		return FText::FromString(TEXT("I"));

	case ECombatRoomRewardType::Weapon:
		return FText::FromString(TEXT("W"));

	default:
		return FText::FromString(TEXT("?"));
	}
}
