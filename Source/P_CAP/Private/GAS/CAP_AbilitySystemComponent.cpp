// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CAP_AbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GameplayEffectExtension.h"
#include "Setting/CAP_AttributeSet.h"
#include "Setting/CAP_GameplayAbilityTypes.h"

UCAP_AbilitySystemComponent::UCAP_AbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UCAP_AttributeSet::GetHealthAttribute()).AddUObject(this, &UCAP_AbilitySystemComponent::HealthUpdated);
}

void UCAP_AbilitySystemComponent::InitComponent()
{
	ApplyInitialEffects();
	InitialBaseAttribute();
	GiveInitialAbilities();
}

void UCAP_AbilitySystemComponent::ApplyInitialEffects()
{
	if (!AbilitySystemGenerics || !GetOwner())
		return;

	for (const TSubclassOf<UGameplayEffect>& Effect : AbilitySystemGenerics->GetInitialEffects())
	{
		FGameplayEffectSpecHandle EffectSpec = MakeOutgoingSpec(Effect, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
	}
}

void UCAP_AbilitySystemComponent::InitialBaseAttribute()
{
	if (!GetOwner())
		return;
	const FBaseStatRow* BastStats = nullptr;
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
	}
}

void UCAP_AbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner())
		return;

	bool bFound = false;
	float MaxHealth = GetGameplayAttributeValue(UCAP_AttributeSet::GetMaxHealthAttribute(), bFound);
	//변경된 HP값이 최대 체력보다 높을 경우
	if (bFound && ChangeData.NewValue >= MaxHealth)
	{
		// 풀피 태그가 없었던 경우라면 풀피 태그 부여
		if (!HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(/*체력 만땅 상태 태그*/"")))
		{
			AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(/*체력 만땅 상태 태그*/""));
		}
	}
	//변경된 HP값이 최대 체력보다 낮은 경우 풀피 태그 제거
	else
	{
		RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(/*체력 만땅 상태 태그*/""));
	}

	// 변경되는 HP값이 0보다 낮아지는 경우
	if (ChangeData.NewValue <= 0.f)
	{
		// 죽었다는 태그가 없으면 죽음 태그 부여
		if (!HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(/*체력 빈 상태 태그*/"")))
		{
			AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(/*체력 빈 상태 태그*/""));
		}
		
		if (AbilitySystemGenerics && AbilitySystemGenerics->GetDeathEffect())
		{
			ApplyGameplayEffect(AbilitySystemGenerics->GetDeathEffect());
		}
		FGameplayEventData DeadAbilityEventData;
		if (ChangeData.GEModData)
			DeadAbilityEventData.ContextHandle = ChangeData.GEModData->EffectSpec.GetContext();
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), FGameplayTag::RequestGameplayTag(/*죽음태그*/""), DeadAbilityEventData);
	}
	else
	{
		RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(""));
	}
}
