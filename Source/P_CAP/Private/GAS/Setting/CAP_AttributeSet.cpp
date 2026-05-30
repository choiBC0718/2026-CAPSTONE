// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting/CAP_AttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UCAP_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetMaxHealthAttribute())
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
}

void UCAP_AttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		float CurrentMaxHealth = GetMaxHealth();
		if (CurrentMaxHealth > 0.f)
			NewValue = FMath::Clamp(NewValue, 0.f, CurrentMaxHealth);
	}
}

void UCAP_AttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* Target = GetOwningActor();
	AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
	if (!bIsOwnerCached && Target)
	{
		bIsPlayerOwner = (Cast<ACAP_PlayerCharacter>(Target) != nullptr);
		bIsOwnerCached = true;
	}
	if (!CachedProgressionSubsystem && Target)
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(Target))
			CachedProgressionSubsystem = GI->GetSubsystem<UCAP_ProgressionSubsystem>();
	}
	
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		float LocalDamage = GetDamage();
		SetDamage(0.f);
		if (LocalDamage > 0.f)
		{
			if (CachedProgressionSubsystem)
			{
				if (bIsPlayerOwner)	// 플레이어가 맞은경우 (Target == Player)
					CachedProgressionSubsystem->AddDamageTaken(LocalDamage);
				else if (Instigator && Instigator->IsA<ACAP_PlayerCharacter>())	// 플레이어가 때린 경우 (Instigator == Player)
					CachedProgressionSubsystem->AddDamageDeal(LocalDamage);
			}
			
			if (GetShield() > 0.f)
			{
				float ShieldDamage = FMath::Min(GetShield(), LocalDamage);
				SetShield(GetShield() - ShieldDamage);
				LocalDamage -= ShieldDamage;
			}
			if (LocalDamage >0.f)
			{
				const float NewHealth = GetHealth() - LocalDamage;
				SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
			}
		}
	}
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float DeltaValue = Data.EvaluatedData.Magnitude;
		if (DeltaValue > 0.f && CachedProgressionSubsystem && bIsPlayerOwner)
			CachedProgressionSubsystem->AddHealing(DeltaValue);
		
		SetHealth(FMath::Clamp(GetHealth(), 0, GetMaxHealth()));
	}
}

void UCAP_AttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
	const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	const float CurrentMaxVal = MaxAttribute.GetCurrentValue();

	if (CurrentMaxVal <=0.f || !ASC) return;
	if (!FMath::IsNearlyEqual(CurrentMaxVal, NewMaxValue) && ASC)
	{
		const float CurrentVal = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxVal > 0.f)? (CurrentVal * NewMaxValue / CurrentMaxVal)-CurrentVal : NewMaxValue;
		ASC->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}