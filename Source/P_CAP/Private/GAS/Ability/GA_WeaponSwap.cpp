// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GA_WeaponSwap.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"

UGA_WeaponSwap::UGA_WeaponSwap()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}


void UGA_WeaponSwap::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo())
	{
		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
		{
			WeaponComp->SwapWeapon();
			ApplySwapCooldown();
		}
	}
	K2_EndAbility();
}
bool UGA_WeaponSwap::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (SwapCooldownTag.IsValid())
	{
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(SwapCooldownTag))
		{
			if (OptionalRelevantTags)
				OptionalRelevantTags->AddTag(SwapCooldownTag);
			return false;
		}
	}
	return true;
}

void UGA_WeaponSwap::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
}

void UGA_WeaponSwap::ApplySwapCooldown()
{
	UCAP_AbilitySystemComponent* CAP_ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!CAP_ASC || !CAP_ASC->GetGenerics() || !CAP_ASC->GetGenerics()->GetCooldownEffect())
		return;

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CAP_ASC->GetGenerics()->GetCooldownEffect(), GetAbilityLevel());
	
	if (SpecHandle.IsValid())
	{
		FGameplayTag DataCooldownTag = UCAP_AbilitySystemStatics::GetDataCooldownTag();
		SpecHandle.Data->SetSetByCallerMagnitude(DataCooldownTag, SwapCooldownTime);
		
		if (SwapCooldownTag.IsValid())
			SpecHandle.Data->DynamicGrantedTags.AddTag(SwapCooldownTag);

		CAP_ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
