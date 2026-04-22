// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_AbilitySlot.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_AbilitySlotData.h"

void UCAP_AbilitySlot::NativeConstruct()
{
	Super::NativeConstruct();
	
	CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnerASC)
	{
		OwnerASC->AbilityCommittedCallbacks.AddUObject(this, &UCAP_AbilitySlot::AbilityCommitted);
	}
	
	WholeNumberFormattingOptions.MaximumFractionalDigits = 0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits = 1;
}

void UCAP_AbilitySlot::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UCAP_AbilitySlotData* SlotData = Cast<UCAP_AbilitySlotData>(ListItemObject);
	if (SlotData && Icon)
	{
		CooldownTag = SlotData->CooldownTag;
		
		UTexture2D* LoadedIcon = SlotData->SkillIcon.LoadSynchronous();
		if (LoadedIcon)
		{
			if (UMaterialInstanceDynamic* MID = Icon->GetDynamicMaterial())
			{
				MID->SetTextureParameterValue(IconMaterialParamName, LoadedIcon);
			}
		}
	}
}

void UCAP_AbilitySlot::AbilityCommitted(UGameplayAbility* Ability)
{
	if (!OwnerASC || !CooldownTag.IsValid())
		return;
	
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(CooldownTag));
	TArray<float> Durations = OwnerASC->GetActiveEffectsDuration(Query);
	TArray<float> Remainings = OwnerASC->GetActiveEffectsTimeRemaining(Query);
	
	if (Remainings.Num() > 0)
	{
		float MaxRemaining = 0.f;
		float MaxDuration = 0.f;
		for (int32 i = 0; i < Remainings.Num(); ++i)
		{
			if (Remainings[i] > MaxRemaining)
			{
				MaxRemaining = Remainings[i];
				MaxDuration = Durations.IsValidIndex(i) ? Durations[i] : 0.f;
			}
		}
		if (MaxRemaining > 0.f)
			StartCooldown(MaxRemaining, MaxDuration);
	}
}

void UCAP_AbilitySlot::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	CachedCooldownDuration = CooldownDuration;
	CachedCooldownTimeRemaining = CooldownTimeRemaining;

	CooldownDurationText->SetVisibility(ESlateVisibility::Visible);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UCAP_AbilitySlot::CooldownFinished, CachedCooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerUpdateHandle,this, &UCAP_AbilitySlot::UpdateCooldown, CooldownUpdateInterval,true, 0.f);
}

void UCAP_AbilitySlot::CooldownFinished()
{
	CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;
	CooldownDurationText -> SetVisibility(ESlateVisibility::Hidden);
	GetWorld() -> GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
	Icon -> GetDynamicMaterial()-> SetScalarParameterValue(CooldownPercentParamName, 1.f);
}

void UCAP_AbilitySlot::UpdateCooldown()
{
	CachedCooldownTimeRemaining -= CooldownUpdateInterval;
	CachedCooldownTimeRemaining = FMath::Max(0.f, CachedCooldownTimeRemaining);
	
	FNumberFormattingOptions* FormatOps = CachedCooldownTimeRemaining > 1 ? &WholeNumberFormattingOptions : &TwoDigitNumberFormattingOptions;
	CooldownDurationText->SetText(FText::AsNumber(CachedCooldownTimeRemaining, FormatOps));
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamName, 1.f - CachedCooldownTimeRemaining / CachedCooldownDuration);
}
