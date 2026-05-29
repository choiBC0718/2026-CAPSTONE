// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Library/CAP_WidgetHelper.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UCAP_WidgetHelper::ApplyCustomButtonStyle(class UButton* Button, class UTextBlock* TextBlock, bool bIsFocused,
                                               const FButtonVisualSettings& Settings)
{
	if (!Button || !TextBlock) return;
	
	Button->SetBackgroundColor(FLinearColor::White);

	FButtonStyle Style = Button->GetStyle();
	FSlateFontInfo FontInfo = TextBlock->GetFont();

	Style.Pressed.TintColor = Settings.PressedBgColor;
	Style.Pressed.OutlineSettings.Color = Settings.PressedOutlineColor;
	Style.Pressed.OutlineSettings.Width = Settings.PressedOutlineWidth;
	
	if (bIsFocused)
	{
		Style.Normal.TintColor = Settings.HoverBgColor;
		Style.Hovered.TintColor = Settings.HoverBgColor;

		Style.Normal.OutlineSettings.Color = Settings.HoverOutlineColor;
		Style.Normal.OutlineSettings.Width = Settings.HoverOutlineWidth;
		Style.Hovered.OutlineSettings.Color = Settings.HoverOutlineColor;
		Style.Hovered.OutlineSettings.Width = Settings.HoverOutlineWidth;

		FontInfo.Size = Settings.HoverFontSize;
	}
	else
	{
		Style.Normal.TintColor = Settings.NormalBgColor;
		Style.Hovered.TintColor = Settings.HoverBgColor; 
		
		Style.Normal.OutlineSettings.Color = Settings.NormalOutlineColor;
		Style.Normal.OutlineSettings.Width = Settings.NormalOutlineWidth;
		Style.Hovered.OutlineSettings.Color = Settings.NormalOutlineColor;
		Style.Hovered.OutlineSettings.Width = Settings.NormalOutlineWidth;

		FontInfo.Size = Settings.NormalFontSize;
	}

	Button->SetStyle(Style);
	TextBlock->SetFont(FontInfo);
}
