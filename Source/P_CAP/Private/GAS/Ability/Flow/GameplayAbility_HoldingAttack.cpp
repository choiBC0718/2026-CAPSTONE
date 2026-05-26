// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Flow/GameplayAbility_HoldingAttack.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/Tasks/AbilityTask_TickMoveToCursor.h"

UGameplayAbility_HoldingAttack::UGameplayAbility_HoldingAttack()
{
	JogLoopTag = FGameplayTag::RequestGameplayTag("Ability.Event.Hold.StartLoop");
	MoveToCursor = nullptr;
	MaxHoldTask = nullptr;
	InputReleaseTask = nullptr;
}

void UGameplayAbility_HoldingAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,
                                                     const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive())
		return;
	
	bIsExecuted=false;
	if (MoveToCursor)
	{
		MoveToCursor->EndTask();
		MoveToCursor = nullptr;
	}

	UAbilityTask_WaitGameplayEvent* JogFwdTagTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,JogLoopTag);
	JogFwdTagTask->EventReceived.AddDynamic(this, &UGameplayAbility_HoldingAttack::OnJogFwdTagReceived);
	JogFwdTagTask->ReadyForActivation();

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	InputReleaseTask->OnRelease.AddDynamic(this, &UGameplayAbility_HoldingAttack::OnInputReleased);
	InputReleaseTask->ReadyForActivation();
}

void UGameplayAbility_HoldingAttack::OnInputReleased(float TimeHeld)
{
	ExecuteAttack();
}

void UGameplayAbility_HoldingAttack::OnMaxHeld()
{
	ExecuteAttack();
}

void UGameplayAbility_HoldingAttack::ExecuteAttack()
{
	if (bIsExecuted)
		return;
	
	ClearTasks();
	bIsExecuted=true;

	UAnimInstance* AnimInst = GetOwnerAnimInstance();
	if (AnimInst)
	{
		if (UAnimMontage* AbilityMontage = AnimInst->GetCurrentActiveMontage())
			AnimInst->Montage_JumpToSection(FName("Attack"), AbilityMontage);
	}
}

void UGameplayAbility_HoldingAttack::OnJogFwdTagReceived(FGameplayEventData Payload)
{
	MaxHoldTask = UAbilityTask_WaitDelay::WaitDelay(this,2.f);
	MaxHoldTask->OnFinish.AddDynamic(this, &UGameplayAbility_HoldingAttack::OnMaxHeld);
	MaxHoldTask->ReadyForActivation();
	
	MoveToCursor = UAbilityTask_TickMoveToCursor::MoveToCursor(this);
	MoveToCursor->ReadyForActivation();
}

void UGameplayAbility_HoldingAttack::ClearTasks()
{
	if (MoveToCursor)
	{
		MoveToCursor->EndTask();
		MoveToCursor=nullptr;
	}
	if (MaxHoldTask)
	{
		MaxHoldTask->EndTask();
		MaxHoldTask=nullptr;
	}
	if (InputReleaseTask)
	{
		InputReleaseTask->EndTask();
		InputReleaseTask=nullptr;
	}
}

