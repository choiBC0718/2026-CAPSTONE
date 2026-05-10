// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Payload/GA_PayloadBase.h"

#include "AbilitySystemComponent.h"

bool UGA_PayloadBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                         const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags))
		return false;
	// 가져오는 Spec은 Payload클래스의 스펙 (InputID == 101, 102, 103)
	FGameplayAbilitySpec* PayloadSpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!PayloadSpec)
		return false;
	
	// 기본공격 (1), Skill (2,3)에 대해서만 실행하도록
	if (PayloadSpec->InputID < 100)
		return true;

	int32 ParentInputID = PayloadSpec->InputID - 100;
	// Flow에 의해 실행중인 Ability의 Spec의 InputID 비교하여 같은 것만 실행
	for (const FGameplayAbilitySpec& Spec : ActorInfo->AbilitySystemComponent->GetActivatableAbilities())
		if (Spec.InputID == ParentInputID)
			return Spec.IsActive();

	return false;
}
