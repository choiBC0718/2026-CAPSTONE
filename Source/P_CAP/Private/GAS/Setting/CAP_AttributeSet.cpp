// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting/CAP_AttributeSet.h"

#include "GameplayEffectExtension.h"

void UCAP_AttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
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
		SetCachedHealthPercent(GetHealth()/GetMaxHealth());
	}
}

void UCAP_AttributeSet::RescaleHealth()
{
	if (!GetOwningActor())
		return;
	if (GetCachedHealthPercent() != 0.f && GetHealth() != 0.f)
	{
		SetHealth(GetMaxHealth() * GetCachedHealthPercent());
	}
}
