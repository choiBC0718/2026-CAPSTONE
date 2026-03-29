// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_GameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Weapon/CAP_WeaponInstance.h"


UCAP_GameplayAbility::UCAP_GameplayAbility()
{
	DamageTag = UCAP_AbilitySystemStatics::GetDamageTag();
}

bool UCAP_GameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (AbilitySpec && AbilitySpec->Level <= 0)
		return false;
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UCAP_GameplayAbility::ApplyGameplayEffectToHitResult(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, Level);
	FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	EffectContext.AddHitResult(HitResult);
	EffectSpecHandle.Data->SetContext(EffectContext);
	
	ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
}

UAnimInstance* UCAP_GameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
	if (OwnerSkeletalMeshComp)
	{
		return OwnerSkeletalMeshComp->GetAnimInstance();
	}
	return nullptr;
}

class ACAP_PlayerCharacter* UCAP_GameplayAbility::GetPlayerCharacterFromActorInfo() const
{
	return Cast<ACAP_PlayerCharacter>(GetAvatarActorFromActorInfo());
}

class UCAP_WeaponDataAsset* UCAP_GameplayAbility::GetWeaponDataAsset() const
{
	if (ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo())
	{
		if (UCAP_WeaponInstance* WeaponInst = Player->GetCurrentWeaponInstance())
		{
			return WeaponInst->GetWeaponDA();
		}
	}
	return nullptr;
}

const struct FWeaponSkillData* UCAP_GameplayAbility::GetCurrentSkillData() const
{
	ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo();
	if (!Player)		return nullptr;

	UCAP_WeaponInstance* WeaponInst = Player->GetCurrentWeaponInstance();
	if (!WeaponInst)	return nullptr;
	
	UCAP_WeaponDataAsset* WeaponDA = GetWeaponDataAsset();
	if (!WeaponDA)		return nullptr;

	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (!Spec)			return nullptr;

	// 평타인 경우 DA의 BasicAbility 반환
	if (Spec->InputID == static_cast<int32>(EAbilityInputID::BasicAttack))
	{
		return &WeaponDA->BasicAbility;
	}
	// 스킬인 경우 WeaponInstance가 랜덤으로 뽑은 배열에서 반환
	else if (Spec->InputID >= static_cast<int32>(EAbilityInputID::Skill1))
	{
		int32 SkillIndex = Spec->InputID - static_cast<int32>(EAbilityInputID::Skill1);
		if (WeaponInst->GetGrantedSkills().IsValidIndex(SkillIndex))
		{
			return &WeaponInst->GetGrantedSkills()[SkillIndex];
		}
	}

	return nullptr;
}
