// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting/CAP_AttributeSet.h"

#include "GameplayEffectExtension.h"

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

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		float LocalDamage = GetDamage();
		SetDamage(0.f);
		if (LocalDamage > 0.f)
		{
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