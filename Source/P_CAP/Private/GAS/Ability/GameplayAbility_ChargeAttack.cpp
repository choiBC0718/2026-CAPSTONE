  // Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_ChargeAttack.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Tasks/AbilityTask_TickRotToCursor.h"

UGameplayAbility_ChargeAttack::UGameplayAbility_ChargeAttack()
{
	ChargeStartTag = FGameplayTag::RequestGameplayTag("Ability.Event.Charge.StartLoop");
}

void UGameplayAbility_ChargeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
 	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilityTask_WaitGameplayEvent* WaitChargeStartTag = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ChargeStartTag);
	WaitChargeStartTag->EventReceived.AddDynamic(this, &UGameplayAbility_ChargeAttack::OnChargeStartTagReceived);
	WaitChargeStartTag->ReadyForActivation();

	UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
  	InputReleaseTask->OnRelease.AddDynamic(this, &UGameplayAbility_ChargeAttack::OnInputReleased);
  	InputReleaseTask->ReadyForActivation();
	
  	TickRotTask = UAbilityTask_TickRotToCursor::TickRotToCursor(this, 100.f);
  	TickRotTask->ReadyForActivation();
}

void UGameplayAbility_ChargeAttack::OnChargeStartTagReceived(FGameplayEventData Payload)
{

  	UAbilityTask_WaitDelay* MaxChargeTask = UAbilityTask_WaitDelay::WaitDelay(this,3.f);
  	MaxChargeTask->OnFinish.AddDynamic(this, &UGameplayAbility_ChargeAttack::OnMaxCharged);
  	MaxChargeTask->ReadyForActivation();
}

void UGameplayAbility_ChargeAttack::OnInputReleased(float TimeHeld)
{
  	ExecuteAttack();
}

void UGameplayAbility_ChargeAttack::OnMaxCharged()
{
  	ExecuteAttack();
}

void UGameplayAbility_ChargeAttack::ExecuteAttack()
{
  	if (bIsExecuted)
  		return;
  	if (TickRotTask)
  	{
  		TickRotTask->EndTask();
  		TickRotTask = nullptr;
  	}
  	bIsExecuted = true;
  	
  	UAnimInstance* AnimInst = GetOwnerAnimInstance();
  	if (AnimInst)
  	{
  		if (UAnimMontage* AbilityMontage = AnimInst->GetCurrentActiveMontage())
  			AnimInst->Montage_JumpToSection(FName("Attack"), AbilityMontage);
  	}
}

