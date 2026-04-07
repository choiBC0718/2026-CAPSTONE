// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAP_AbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GameplayEffectExtension.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Setting/CAP_AbilitySystemStatics.h"
#include "Setting/CAP_AttributeSet.h"
#include "Setting/CAP_GameplayAbilityTypes.h"

UCAP_AbilitySystemComponent::UCAP_AbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UCAP_AttributeSet::GetHealthAttribute()).AddUObject(this, &UCAP_AbilitySystemComponent::HealthUpdated);
}

void UCAP_AbilitySystemComponent::InitComponent(FName StatRowName)
{
	if (!AbilitySystemGenerics)
		return;
	InitializeBaseAttribute(StatRowName);
	ApplyInitialEffects();
	GiveInitialAbilities();
}

void UCAP_AbilitySystemComponent::ApplyFullStatEffect()
{
	if (!AbilitySystemGenerics)
		return;
	ApplyGameplayEffect(AbilitySystemGenerics->GetFullStatEffect());
}

void UCAP_AbilitySystemComponent::ApplyInitialEffects()
{
	if (!AbilitySystemGenerics || !GetOwner())
		return;
	
	for (const TSubclassOf<UGameplayEffect>& Effect : AbilitySystemGenerics->GetInitialEffects())
	{
		ApplyGameplayEffect(Effect);
	}
}

void UCAP_AbilitySystemComponent::InitializeBaseAttribute(FName StatRowName)
{
	if (!GetOwner())
		return;
	
	if (!AbilitySystemGenerics->GetBaseStatDataTable())
		return;
	
	const UDataTable* TableToUse =TableToUse = AbilitySystemGenerics->GetBaseStatDataTable();	
	const FBaseStatRow* BaseStats = TableToUse->FindRow<FBaseStatRow>(StatRowName,"");
	if (BaseStats)
	{
		SetNumericAttributeBase(UCAP_AttributeSet::GetMaxHealthAttribute(), BaseStats->BaseMaxHealth);
		SetNumericAttributeBase(UCAP_AttributeSet::GetPhysicalDamageAttribute(), BaseStats->BasePhysicalDamage);
		SetNumericAttributeBase(UCAP_AttributeSet::GetPhysicalPenetrationAttribute(), BaseStats->BasePhysicalPenetration);
		SetNumericAttributeBase(UCAP_AttributeSet::GetMagicalDamageAttribute(), BaseStats->BaseMagicalDamage);
		SetNumericAttributeBase(UCAP_AttributeSet::GetMagicalPenetrationAttribute(), BaseStats->BaseMagicalPenetration);
		SetNumericAttributeBase(UCAP_AttributeSet::GetPhysicalArmorAttribute(), BaseStats->BasePhysicalArmor);
		SetNumericAttributeBase(UCAP_AttributeSet::GetMagicalArmorAttribute(), BaseStats->BaseMagicalArmor);
		SetNumericAttributeBase(UCAP_AttributeSet::GetCriticalChanceAttribute(), BaseStats->BaseCriticalChance);
		SetNumericAttributeBase(UCAP_AttributeSet::GetCriticalDamageAttribute(), BaseStats->BaseCriticalDamage);
		SetNumericAttributeBase(UCAP_AttributeSet::GetMoveSpeedAttribute(), BaseStats->BaseMoveSpeed);
	}
}

void UCAP_AbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner())
		return;

	for (const TPair<EAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicAbility)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, (int32)AbilityPair.Key));
	}
	for (const TPair<EAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0, (int32)AbilityPair.Key));
	}

	if (!AbilitySystemGenerics)
		return;
	for (const TSubclassOf<UGameplayAbility>& PassiveAbility : AbilitySystemGenerics->GetPassiveAbilities())
	{
		GiveAbility(FGameplayAbilitySpec(PassiveAbility, 1, -1));
	}
}

void UCAP_AbilitySystemComponent::ApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level)
{
	if (GetOwner())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UCAP_AbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner())
		return;

	bool bFound = false;
	float MaxHealth = GetGameplayAttributeValue(UCAP_AttributeSet::GetMaxHealthAttribute(), bFound);

	if (bFound && ChangeData.NewValue >= MaxHealth)
	{
		if (!HasMatchingGameplayTag(UCAP_AbilitySystemStatics::GetHealthFullStatTag()))
		{
			AddLooseGameplayTag(UCAP_AbilitySystemStatics::GetHealthFullStatTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UCAP_AbilitySystemStatics::GetHealthFullStatTag());
	}

	if (ChangeData.NewValue <= 0.f)
	{
		if (!HasMatchingGameplayTag(UCAP_AbilitySystemStatics::GetHealthEmptyStatTag()))
		{
			AddLooseGameplayTag(UCAP_AbilitySystemStatics::GetHealthEmptyStatTag());
		}
		
		if (AbilitySystemGenerics && AbilitySystemGenerics->GetDeathEffect())
		{
			ApplyGameplayEffect(AbilitySystemGenerics->GetDeathEffect());
		}
		FGameplayEventData DeadAbilityEventData;
		if (ChangeData.GEModData)
			DeadAbilityEventData.ContextHandle = ChangeData.GEModData->EffectSpec.GetContext();
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), UCAP_AbilitySystemStatics::GetDeadStateTag(), DeadAbilityEventData);
	}
	else
	{
		RemoveLooseGameplayTag(UCAP_AbilitySystemStatics::GetHealthEmptyStatTag());
	}
}
