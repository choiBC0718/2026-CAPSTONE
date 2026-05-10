// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Flow/GA_FlowBase.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AN_SendRMSEvent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GameFramework/RootMotionSource.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "GAS/Tasks/AbilityTask_RotateToCursor.h"

UGA_FlowBase::UGA_FlowBase()
{
}

void UGA_FlowBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	if (!SkillData || !SkillData->AbilityMontage.Get())
	{
		K2_EndAbility();
		return;
	}
	
	ChargedTime = 1.f;

	float AttackSpeed = 1.f;
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		AttackSpeed = ASC->GetNumericAttribute(UCAP_AttributeSet::GetAttackSpeedAttribute());

	// Ability 시전 시, 마우스 방향으로 회전
	UAbilityTask_RotateToCursor* RotToCursor = UAbilityTask_RotateToCursor::SmoothRotateToMouse(this, RotateToCursorSpeed);
	RotToCursor->ReadyForActivation();

	// Ability Montage 실행
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillData->AbilityMontage.Get(),AttackSpeed);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_FlowBase::K2_EndAbility);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_FlowBase::K2_EndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_FlowBase::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_FlowBase::K2_EndAbility);
	MontageTask->ReadyForActivation();

	// 가짜 루트 모션 이벤트 처리
	UAbilityTask_WaitGameplayEvent* WaitRMSTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, RMSTag);
	WaitRMSTask->EventReceived.AddDynamic(this, &UGA_FlowBase::OnRMSTagReceived);
	WaitRMSTask->ReadyForActivation();

	// 기본공격인지 스킬인지 Cast 트리거 전송
	BroadcastTriggerEvent(IsBasicAttack()?TriggerCastBasicTag:TriggerCastAbilityTag);
}

void UGA_FlowBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
		return;
	UCAP_AbilitySystemComponent* CAP_ASC = Cast<UCAP_AbilitySystemComponent>(ASC);
	if (!CAP_ASC || !CAP_ASC->GetGenerics() || !CAP_ASC->GetGenerics()->GetCooldownEffect())
		return;

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CAP_ASC->GetGenerics()->GetCooldownEffect(),GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		const FWeaponSkillData* SkillData = GetSkillDataFromContext(Handle, ActorInfo);
		
		float BaseCooldown = SkillData ? SkillData->CooldownTime : 0.f;
		FGameplayTag CooldownTag = SkillData ? SkillData->CooldownTag : FGameplayTag();

		float CooldownSpeed = ASC->GetNumericAttribute(UCAP_AttributeSet::GetSkillCooldownSpeedAttribute());
		CooldownSpeed = FMath::Max<float>(0.1f, CooldownSpeed);
		float FinalCooldown = BaseCooldown / CooldownSpeed;

		SpecHandle.Data->SetSetByCallerMagnitude(DataCooldownTag, FinalCooldown);
		if (CooldownTag.IsValid())
		{
			SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);
		}
		ApplyGameplayEffectSpecToOwner(Handle,ActorInfo,ActivationInfo,SpecHandle);
	}
}

bool UGA_FlowBase::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	const FWeaponSkillData* Data = GetSkillDataFromContext(Handle, ActorInfo);
	
	if (Data && Data->CooldownTag.IsValid())
	{
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(Data->CooldownTag))
		{
			if (OptionalRelevantTags)
			{
				OptionalRelevantTags->AddTag(Data->CooldownTag);
			}
			return false;
		}
	}
	return true;
}

void UGA_FlowBase::OnRMSTagReceived(FGameplayEventData Payload)
{
	const UAN_SendRMSEvent* RMSNotify = Cast<UAN_SendRMSEvent>(Payload.OptionalObject);
	if (!RMSNotify)
		return;
	
	if (ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo())
	{
		FVector ForwardDir = Player->GetActorForwardVector();
		UAbilityTask_ApplyRootMotionConstantForce* RMTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this, NAME_None,ForwardDir, RMSNotify->RMSStrength, RMSNotify->RMSDuration,false, nullptr,ERootMotionFinishVelocityMode::SetVelocity,
			FVector::ZeroVector, 0.f, false);
		RMTask->ReadyForActivation();
	}
}