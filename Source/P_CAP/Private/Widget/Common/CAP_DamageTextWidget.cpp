// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_DamageTextWidget.h"

#include "Components/TextBlock.h"

void UCAP_DamageTextWidget::PlayDamageAnimation(float DamageAmount, bool bIsCritical, bool bIsPlayer)
{
	if (DamageText)
	{
		int32 RoundedDamage = FMath::RoundToInt(DamageAmount);
		DamageText->SetText(FText::AsNumber(RoundedDamage));
	}

	UWidgetAnimation* AnimToPlay = nullptr;

	if (bIsPlayer)
	{
		AnimToPlay = PlayerDamageAnim;
	}
	else if (bIsCritical)
	{
		AnimToPlay = CriticalDamageAnim;
	}
	else
	{
		AnimToPlay = NormalDamageAnim;
	}
	
	if (AnimToPlay)
	{
		PlayAnimation(AnimToPlay);
	}
	else
	{
		OnDamageAnimFinished.Broadcast();
	}
}

void UCAP_DamageTextWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);
	OnDamageAnimFinished.Broadcast();
}
