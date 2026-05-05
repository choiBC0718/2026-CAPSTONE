// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotWidgets/CAP_AbilitySlot.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCAP_AbilitySlot::NativeConstruct()
{
	Super::NativeConstruct();
	
	CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
	if (!OwnerASC)
	{
		OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
		if (OwnerASC)
			OwnerASC->AbilityCommittedCallbacks.AddUObject(this, &UCAP_AbilitySlot::AbilityCommitted);
	}
	
	WholeNumberFormattingOptions.MaximumFractionalDigits = 0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits = 1;
	CheckCurrentCooldown();
}

void UCAP_AbilitySlot::NativeDestruct()
{
	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearAllTimersForObject(this);
	if (OwnerASC)
		OwnerASC->AbilityCommittedCallbacks.RemoveAll(this);
	Super::NativeDestruct();
}

void UCAP_AbilitySlot::InitSlot(const FWeaponSkillData& SkillData, bool bIsActive)
{
	CooldownTag = SkillData.CooldownTag;
	if (UTexture2D* LoadedIcon = SkillData.SkillIcon.LoadSynchronous())
	{
		if (UMaterialInstanceDynamic* MID = Icon->GetDynamicMaterial())
			MID->SetTextureParameterValue(IconMaterialParamName, LoadedIcon);
	}
	if (bIsActive)
	{
		SetRenderScale(FVector2D(1.0f, 1.0f));
		Icon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	}
	else
	{
		SetRenderScale(SubSkillSize);
		Icon->SetColorAndOpacity(SubSkillColor);
	}

	if (!OwnerASC)
	{
		OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
		if (OwnerASC)
		{
			// AbilityCommittedCallbacks는 ActivateAbility가 실행될 때 무조건 호출됨 -> AbilityCommitted 무조건 1회 실행 -> 쿨타임 타이머 돌떄는 패스 하도록 설정 필요
			OwnerASC->AbilityCommittedCallbacks.RemoveAll(this);
			OwnerASC->AbilityCommittedCallbacks.AddUObject(this, &UCAP_AbilitySlot::AbilityCommitted);
		}
	}
	CheckCurrentCooldown();
}

void UCAP_AbilitySlot::AbilityCommitted(UGameplayAbility* Ability)
{
	CheckCurrentCooldown();
}

void UCAP_AbilitySlot::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	// 이미 타이머가 작동하고 있고 오차 범위 내에선 재시작하지 않도록
	if (GetWorld()->GetTimerManager().IsTimerActive(CooldownTimerUpdateHandle))
	{
		if (FMath::IsNearlyEqual(CachedCooldownTimeRemaining, CooldownTimeRemaining,0.15f))
			return;
	}
	
	CachedCooldownDuration = CooldownDuration;
	CachedCooldownTimeRemaining = CooldownTimeRemaining;
	FNumberFormattingOptions* FormatOps = CachedCooldownTimeRemaining > 1 ? &WholeNumberFormattingOptions : &TwoDigitNumberFormattingOptions;
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration,FormatOps));

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

void UCAP_AbilitySlot::CheckCurrentCooldown()
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
