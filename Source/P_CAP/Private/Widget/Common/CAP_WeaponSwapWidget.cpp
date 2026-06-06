// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_WeaponSwapWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CAP_ItemInteraction.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/Image.h"
#include "Framework/CAP_GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UCAP_WeaponSwapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (Player)
	{
		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
			WeaponComp->OnWeaponChanged.AddDynamic(this, &UCAP_WeaponSwapWidget::HandleWeaponSwaped);
	}
	
	if (!OwnerASC)
	{
		OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);
		if (OwnerASC)
			OwnerASC->AbilityCommittedCallbacks.AddUObject(this, &UCAP_WeaponSwapWidget::AbilityCommitted);
		if (CooldownTag.IsValid())
		{
			OwnerASC->RegisterGameplayTagEvent(CooldownTag,EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UCAP_WeaponSwapWidget::OnCooldownTagChanged);
		}
	}
	CheckCurrentCooldown();
}

void UCAP_WeaponSwapWidget::HandleWeaponSwaped(class UCAP_WeaponInstance* NewWeaponInstance, class UCAP_WeaponInstance* OldWeaponInstance)
{
	SetSwapKeyImg();
	
	if (NewWeaponInstance && CurrentWeaponImg)
	{
		if (UTexture2D* CurrentIcon = NewWeaponInstance->GetWeaponDA()->ItemIcon.LoadSynchronous())
		{
			CurrentWeaponImg->SetBrushFromTexture(CurrentIcon);
		}
	}
	if (!OldWeaponInstance && StandbyWeaponImg)
	{
		StandbyWeaponImg->SetVisibility(ESlateVisibility::Collapsed);
		SwapKeyImg->SetVisibility(ESlateVisibility::Collapsed);
	}
	else if (OldWeaponInstance && StandbyWeaponImg)
	{
		StandbyWeaponImg->SetVisibility(ESlateVisibility::Visible);
		SwapKeyImg->SetVisibility(ESlateVisibility::Visible);
		if (UTexture2D* StanbyIcon = OldWeaponInstance->GetWeaponDA()->ItemIcon.LoadSynchronous())
		{
			if (UMaterialInstanceDynamic* MID = StandbyWeaponImg->GetDynamicMaterial())
				MID->SetTextureParameterValue(IconMaterialParamName,StanbyIcon);

		}
	}
}

void UCAP_WeaponSwapWidget::AbilityCommitted(UGameplayAbility* GameplayAbility)
{
	CheckCurrentCooldown();
}

void UCAP_WeaponSwapWidget::OnCooldownTagChanged(FGameplayTag GameplayTag, int NewCount)
{
	CheckCurrentCooldown();
}

void UCAP_WeaponSwapWidget::CheckCurrentCooldown()
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

void UCAP_WeaponSwapWidget::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	if (GetWorld()->GetTimerManager().IsTimerActive(CooldownTimerUpdateHandle))
	{
		if (FMath::IsNearlyEqual(CachedCooldownTimeRemaining, CooldownTimeRemaining,0.15f))
			return;
	}
	
	CachedCooldownDuration = CooldownDuration;
	CachedCooldownTimeRemaining = CooldownTimeRemaining;
	
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UCAP_WeaponSwapWidget::CooldownFinished, CachedCooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerUpdateHandle,this, &UCAP_WeaponSwapWidget::UpdateCooldown, 0.02f,true, 0.f);
}

void UCAP_WeaponSwapWidget::CooldownFinished()
{
	CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;
	GetWorld() -> GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
	StandbyWeaponImg -> GetDynamicMaterial()-> SetScalarParameterValue(CooldownPercentParamName, 1.f);
}

void UCAP_WeaponSwapWidget::UpdateCooldown()
{
	CachedCooldownTimeRemaining -= 0.02f;
	CachedCooldownTimeRemaining = FMath::Max(0.f, CachedCooldownTimeRemaining);
	
	StandbyWeaponImg->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamName, 1.f - CachedCooldownTimeRemaining / CachedCooldownDuration);
}

void UCAP_WeaponSwapWidget::SetSwapKeyImg()
{
	ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player)
		return;
	
	FName RowName = FName(*Player->GetAbilityKeyName(EAbilityInputID::SwapWeapon));
	
	if (UCAP_GameInstance* GI = Cast<UCAP_GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UDataTable* KeyTable = GI->GetKeyIconTable())
		{
			FKeyIconRow* Row = KeyTable->FindRow<FKeyIconRow>(RowName, ""); 
			if (Row && SwapKeyImg)
				SwapKeyImg->SetBrushFromTexture(Row->Icon);
		}
	}
}
