// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_TargetEffectWidget.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"

void UCAP_TargetEffectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (StackBarImage)
	{
		StackBarImage->SetVisibility(ESlateVisibility::Hidden);
		StackMID = StackBarImage->GetDynamicMaterial();
	}
}

void UCAP_TargetEffectWidget::UpdateEffectUI(const FGameplayTag& BehaviorTag,int32 CurrentStack, int32 MaxStack)
{
	if (CurrentStack>0)
		ActiveStacks.Add(BehaviorTag,{CurrentStack,MaxStack});
	else
		ActiveStacks.Remove(BehaviorTag);

	FGameplayTag PriorityTag;
	int32 MinRemaining = 9999;
	float MaxPercent = -1.f;

	for (const auto& Pair : ActiveStacks)
	{
		int32 RemainingHits = Pair.Value.MaxStack - Pair.Value.CurrentStack;
		float Percent = (float)Pair.Value.CurrentStack / (float)Pair.Value.MaxStack;

		if (RemainingHits<MinRemaining || (RemainingHits==MinRemaining && Percent>MaxPercent))
		{
			MinRemaining = RemainingHits;
			MaxPercent = Percent;
			PriorityTag = Pair.Key;
		}
	}
	if (PriorityTag.IsValid())
	{
		FTargetStackInfo Info = ActiveStacks[PriorityTag];
		StackBarImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		if (StackSizeBox)
			StackSizeBox->SetWidthOverride(Info.MaxStack * WidthPerStack);
		
		if (StackMID)
		{
			StackMID->SetScalarParameterValue(FName("CurrentStack"), Info.CurrentStack);
			StackMID->SetScalarParameterValue(FName("MaxStack"), Info.MaxStack);
		}
	}
	else
	{
		StackBarImage->SetVisibility(ESlateVisibility::Hidden);
	}
}
