// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_ItemEffectSlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_ItemDataAsset.h"
#include "Items/Item/CAP_ItemInstance.h"

void UCAP_ItemEffectSlot::NativeConstruct()
{
	Super::NativeConstruct();
	StackText->SetVisibility(ESlateVisibility::Hidden);
}

void UCAP_ItemEffectSlot::InitSlot(class UCAP_ItemInstance* InItemInst, FGameplayTag InDynamicTag, float InCooldown,
	float InDuration, int32 InStacks)
{
	ItemInst = InItemInst;
	DynamicTag = InDynamicTag;
	MaxCooldown = InCooldown;
	MaxDuration = InDuration;
	RemainingCooldown = InCooldown;
	RemainingDuration = InDuration;
	CurrentStack = InStacks;

	if (!ItemInst || !Icon)
		return;

	if (UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA())
	{
		if (UTexture2D* LoadedIcon = ItemDA->ItemIcon.LoadSynchronous())
		{
			if (UMaterialInstanceDynamic* MID = Icon->GetDynamicMaterial())
				MID->SetTextureParameterValue(IconMaterialParamName, LoadedIcon);
		}
	}
	if (CurrentStack > 0)
	{
		StackText->SetText(FText::Format(FText::FromString("x{0}"), CurrentStack));
		StackText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		StackText->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MaxDuration <0.f)
	{
		if (UMaterialInstanceDynamic* MID = Icon->GetDynamicMaterial())
		{
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
