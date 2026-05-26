// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_ItemEffectSlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCAP_ItemEffectSlot::NativeConstruct()
{
	Super::NativeConstruct();
	StackText->SetVisibility(ESlateVisibility::Hidden);
}

void UCAP_ItemEffectSlot::InitSlot(const FBuffSlotID& InSlotID, const FBuffUIData& InUIData)
{
	SlotID = InSlotID;

	CurrentStack = InUIData.Stacks;
	MaxCooldown = InUIData.MaxCooldown;
	RemainingCooldown = InUIData.RemainingCooldown;
	MaxDuration = InUIData.MaxDuration;
	RemainingDuration = InUIData.RemainingDuration;

	if (!InUIData.Icon.IsNull())
	{
		if (UTexture2D* LoadedIcon = InUIData.Icon.LoadSynchronous())
			if (UMaterialInstanceDynamic* MID = Icon->GetDynamicMaterial())
				MID->SetTextureParameterValue(IconMaterialParamName, LoadedIcon);
	}
	
	if (CurrentStack > 0)
	{
		StackText->SetText(FText::Format(FText::FromString("x{0}"), CurrentStack));
		StackText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
		StackText->SetVisibility(ESlateVisibility::Hidden);
	
	if (MaxDuration <= 0.f && MaxCooldown <= 0.f)
	{	// 타이머 자체를 안켜도 되는 상황
		if (UMaterialInstanceDynamic* MID = Icon->GetDynamicMaterial())
		{	// 쿨다운:1, Duration:0 이어야 보이지 않음
			MID->SetScalarParameterValue(CooldownPercentParamName, 1.f);
			MID->SetScalarParameterValue(DurationPercentParamName, 0.f); 
		}
		return;
	}
	StartTimers();
}

void UCAP_ItemEffectSlot::StartTimers()
{
	GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UCAP_ItemEffectSlot::UpdateGauges, UpdateInterval, true);
}

void UCAP_ItemEffectSlot::UpdateGauges()
{
	bool bIsActive = false;
	if (UMaterialInstanceDynamic* MID = Icon->GetDynamicMaterial())
	{
		if (RemainingCooldown > 0.f)
		{
			RemainingCooldown -= UpdateInterval;
			float Percent = 1.f - FMath::Clamp(RemainingCooldown / MaxCooldown, 0.f, 1.f);
			MID->SetScalarParameterValue(CooldownPercentParamName, Percent);
			bIsActive = true;
		}
		else
		{
			MID->SetScalarParameterValue(CooldownPercentParamName, 1.f);
		}

		if (RemainingDuration > 0.f)
		{
			RemainingDuration -= UpdateInterval;
			float Percent = FMath::Clamp(RemainingDuration / MaxDuration, 0.f, 1.f);
			MID->SetScalarParameterValue(DurationPercentParamName, Percent);
			bIsActive = true;
		}
		else
		{
			MID->SetScalarParameterValue(DurationPercentParamName, 0.f);
		}
	}
	if (!bIsActive)
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
		RemoveFromParent();
	}
}
