// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/Widget/CombatRewardChoiceWidget.h"

#include "Map/NextRoomChoiceManager.h"

void UCombatRewardChoiceWidget::InitializeChoiceWidget(
	ANextRoomChoiceManager* InChoiceManager,
	const TArray<FCombatRewardChoiceOption>& InOptions)
{
	ChoiceManager = InChoiceManager;
	Options = InOptions;
	OnChoiceWidgetInitialized(Options);
}

void UCombatRewardChoiceWidget::ChooseReward(ECombatRoomRewardType RewardType)
{
	if (ChoiceManager)
	{
		ChoiceManager->SelectReward(RewardType);
	}
}

void UCombatRewardChoiceWidget::CloseChoiceWidget()
{
	RemoveFromParent();
}
