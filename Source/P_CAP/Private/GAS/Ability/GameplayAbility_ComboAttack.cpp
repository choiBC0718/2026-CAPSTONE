// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_ComboAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "GAS/Tasks/AbilityTask_RotateToCursor.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"

UGameplayAbility_ComboAttack::UGameplayAbility_ComboAttack()
{
	AbilityTags.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());
	BlockAbilitiesWithTag.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());
	
	ComboChangeTag = FGameplayTag::RequestGameplayTag("Ability.Combo");
	ComboEndTag = FGameplayTag::RequestGameplayTag("Ability.Combo.End");
	RotateTag = FGameplayTag::RequestGameplayTag("Ability.Event.Rotate");
}

void UGameplayAbility_ComboAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitGameplayEvent* WaitNextComboTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ComboChangeTag, nullptr, false, false);
	WaitNextComboTask->EventReceived.AddDynamic(this, &UGameplayAbility_ComboAttack::OnNextComboTagReceived);
	WaitNextComboTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* RotateTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, RotateTag, nullptr, false);
	RotateTask->EventReceived.AddDynamic(this, &UGameplayAbility_ComboAttack::OnRotateTagReceived);
	RotateTask->ReadyForActivation();
	
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


void UGameplayAbility_ComboAttack::HandleInputPress(float TimeWaited)
{
	SetupWaitComoInputPress();
	TryCommitCombo();
}

void UGameplayAbility_ComboAttack::OnRotateTagReceived(FGameplayEventData Payload)
{
	UAbilityTask_RotateToCursor* RotateTask = UAbilityTask_RotateToCursor::SmoothRotateToMouse(this, 1250.f);
	RotateTask->ReadyForActivation();
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
