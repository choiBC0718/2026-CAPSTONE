// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Flow/GameplayAbility_Charge_Slowdown.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Tasks/AbilityTask_TickRotToCursor.h"

UGameplayAbility_Charge_Slowdown::UGameplayAbility_Charge_Slowdown()
{
	ChargeStartTag = FGameplayTag::RequestGameplayTag("Ability.Event.Charge.StartLoop");
}

void UGameplayAbility_Charge_Slowdown::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive())
		return;

	TickRotTask = UAbilityTask_TickRotToCursor::TickRotToCursor(this, TickRotSpeed);
	TickRotTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitChargeStartTag = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ChargeStartTag);
	WaitChargeStartTag->EventReceived.AddDynamic(this, &UGameplayAbility_Charge_Slowdown::OnChargeStartTagReceived);
	WaitChargeStartTag->ReadyForActivation();

	UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	InputReleaseTask->OnRelease.AddDynamic(this, &UGameplayAbility_Charge_Slowdown::OnInputReleased);
	InputReleaseTask->ReadyForActivation();
}

void UGameplayAbility_Charge_Slowdown::OnChargeStartTagReceived(FGameplayEventData Payload)
{
	UAbilityTask_WaitDelay* MaxChargeTask = UAbilityTask_WaitDelay::WaitDelay(this,MaxChargeTime);
	MaxChargeTask->OnFinish.AddDynamic(this, &UGameplayAbility_Charge_Slowdown::OnMaxCharged);
	MaxChargeTask->ReadyForActivation();

	ChangeCurrentMontagePlayRate(MontageSpeedRateAtCharging);
}

void UGameplayAbility_Charge_Slowdown::OnInputReleased(float TimeHeld)
{
	ExecuteAttack(FMath::Max(1.f, TimeHeld));
}

void UGameplayAbility_Charge_Slowdown::OnMaxCharged()
{
	ExecuteAttack(MaxChargeTime);
}

void UGameplayAbility_Charge_Slowdown::ExecuteAttack(float ChargeTime)
{
	if (bIsExecuted)
		return;
	if (TickRotTask)
	{
		TickRotTask->EndTask();
		TickRotTask = nullptr;
	}
	bIsExecuted = true;
	ChargedTime = ChargeTime;
	
	UAnimInstance* AnimInst = GetOwnerAnimInstance();
	if (AnimInst)
	{
		if (UAnimMontage* AbilityMontage = AnimInst->GetCurrentActiveMontage())
			AnimInst->Montage_JumpToSection(FName("Attack"), AbilityMontage);
	}
	ChangeCurrentMontagePlayRate(1.f);
}
