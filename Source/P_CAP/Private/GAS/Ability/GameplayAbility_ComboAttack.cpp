// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_ComboAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"

UGameplayAbility_ComboAttack::UGameplayAbility_ComboAttack()
{
	AbilityTags.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());
	BlockAbilitiesWithTag.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());

	TargetClearTag = UCAP_AbilitySystemStatics::GetTargetClearTag();
	ComboChangeTag = FGameplayTag::RequestGameplayTag("Ability.Combo");
	ComboEndTag = FGameplayTag::RequestGameplayTag("Ability.Combo.End");
}

void UGameplayAbility_ComboAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	const FWeaponSkillData* SkillData = GetCurrentSkillData();
	if (!SkillData || !SkillData->AbilityMontage)
		return;
	
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	AbilityMontage = SkillData->AbilityMontage;
	IgnoreTargets.Empty();
	
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AbilityMontage);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_ComboAttack::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_ComboAttack::K2_EndAbility);
	PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_ComboAttack::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_ComboAttack::K2_EndAbility);
	PlayMontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitNextComboTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ComboChangeTag, nullptr, false, false);
	WaitNextComboTask->EventReceived.AddDynamic(this, &UGameplayAbility_ComboAttack::OnNextComboTagReceived);
	WaitNextComboTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitDamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, DamageTag);
	WaitDamageTask->EventReceived.AddDynamic(this, &UGameplayAbility_ComboAttack::OnDamageTagReceived);
	WaitDamageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitTargetClearTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TargetClearTag);
	WaitTargetClearTask->EventReceived.AddDynamic(this, &UGameplayAbility_ComboAttack::OnTargetClearTagReceived);
	WaitTargetClearTask->ReadyForActivation();

	SetupWaitComoInputPress();
}

void UGameplayAbility_ComboAttack::SetupWaitComoInputPress()
{
	UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this, &UGameplayAbility_ComboAttack::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void UGameplayAbility_ComboAttack::OnNextComboTagReceived(FGameplayEventData Payload)
{
	FGameplayTag EventTag = Payload.EventTag;
	if (EventTag == ComboEndTag)
	{
		NextComboSectionName = NAME_None;
		return;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	NextComboSectionName = TagNames.Last();
}

void UGameplayAbility_ComboAttack::OnDamageTagReceived(FGameplayEventData Payload)
{
	const FWeaponSkillData* SkillData = GetCurrentSkillData();
	if (!SkillData || !SkillData->SkillDamageTypeEffect)
		return;
	
	int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(Payload.TargetData);
	for (int i =0; i<HitResultCount; i++)
	{
		FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Payload.TargetData, i);
		
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(SkillData->SkillDamageTypeEffect, GetAbilityLevel(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()));
		FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
		EffectContext.AddHitResult(HitResult);
		EffectSpecHandle.Data -> SetContext(EffectContext);

		ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
	}
}

void UGameplayAbility_ComboAttack::OnTargetClearTagReceived(FGameplayEventData Payload)
{
	if (Payload.EventTag == TargetClearTag)
	{
		IgnoreTargets.Empty();
	}
}

void UGameplayAbility_ComboAttack::HandleInputPress(float TimeWaited)
{
	SetupWaitComoInputPress();
	TryCommitCombo();
}

void UGameplayAbility_ComboAttack::TryCommitCombo()
{
	if (NextComboSectionName == NAME_None)
		return;

	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (!OwnerAnimInst)
		return;

	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(AbilityMontage), NextComboSectionName, AbilityMontage);
}
