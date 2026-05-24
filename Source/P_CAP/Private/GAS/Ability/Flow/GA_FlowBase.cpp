// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Flow/GA_FlowBase.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AN_SendRMSEvent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GameFramework/RootMotionSource.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "GAS/Tasks/AbilityTask_RotateToCursor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "P_CAP/P_CAP.h"

UGA_FlowBase::UGA_FlowBase()
{
	RMSTag = UCAP_AbilitySystemStatics::GetRMSTag();
	DataCooldownTag = UCAP_AbilitySystemStatics::GetDataCooldownTag();
	
	AnimHitTag = UCAP_AbilitySystemStatics::GetAnimHitTag();
	AnimSpawnTag = UCAP_AbilitySystemStatics::GetAnimSpawnTag();
	
	DoDamageTag = UCAP_AbilitySystemStatics::GetDamageTag();
	SpawnProjectileTag = UCAP_AbilitySystemStatics::GetSpawnProjectileTag();
	
	TriggerCastBasicTag = UCAP_AbilitySystemStatics::GetItemTriggerCastBasic();
	TriggerCastAbilityTag = UCAP_AbilitySystemStatics::GetItemTriggerCastAbility();
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

	// 애님 몽타주에서 Hit타격 시점을 기다림 -> 타격 범위 내 몬스터 데이터를 가지고 Payload클래스 트리거 시킴
	UAbilityTask_WaitGameplayEvent* WaitAnimHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AnimHitTag);
	WaitAnimHitTask->EventReceived.AddDynamic(this, &UGA_FlowBase::OnAnimHitTagReceived);
	WaitAnimHitTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitAnimSpawnTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AnimSpawnTag);
	WaitAnimSpawnTask->EventReceived.AddDynamic(this, &UGA_FlowBase::OnAnimSpawnTagReceived);
	WaitAnimSpawnTask->ReadyForActivation();
}

void UGA_FlowBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsCollisionIgnored)
	{
		if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetAvatarActorFromActorInfo()))
		{
			UCapsuleComponent* Capsule = Player->GetCapsuleComponent();
			FVector EndLoc = Player->GetActorLocation();
			
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(Player);
			FHitResult HitResult;
			
			bool bIsOverlapping = UKismetSystemLibrary::SphereTraceSingle(
				Player, EndLoc, EndLoc, Capsule->GetScaledCapsuleRadius() + 10.f, 
				UEngineTypes::ConvertToTraceType(ECC_Hitbox), false, ActorsToIgnore, 
				EDrawDebugTrace::None, HitResult, true);
			
			if (bIsOverlapping)
			{
				FVector PushDir = (EndLoc - HitResult.ImpactPoint).GetSafeNormal();
				if (PushDir.IsNearlyZero())
					PushDir = -Player->GetActorForwardVector();
				
				FVector SnapLocation = EndLoc + (PushDir * (Capsule->GetScaledCapsuleRadius() * 2.0f));
				Player->SetActorLocation(SnapLocation, false, nullptr, ETeleportType::TeleportPhysics);
			}
			Capsule->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
		}
		bIsCollisionIgnored = false;
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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
		if (RMSNotify->bIgnoreHitboxCollision)
		{
			Player->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Ignore);
			bIsCollisionIgnored = true;
		}
		FVector ForwardDir = Player->GetActorForwardVector();
		UAbilityTask_ApplyRootMotionConstantForce* RMTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this, NAME_None,ForwardDir, RMSNotify->RMSStrength, RMSNotify->RMSDuration,false, nullptr,ERootMotionFinishVelocityMode::SetVelocity,
			FVector::ZeroVector, 0.f, false);
		RMTask->ReadyForActivation();
	}
}

void UGA_FlowBase::OnAnimHitTagReceived(FGameplayEventData Payload)
{
	BroadcastTriggerEvent(DoDamageTag, Payload.TargetData ,ChargedTime);
}

void UGA_FlowBase::OnAnimSpawnTagReceived(FGameplayEventData Payload)
{
	BroadcastTriggerEvent(SpawnProjectileTag, Payload.TargetData ,ChargedTime);
}

void UGA_FlowBase::ChangeCurrentMontagePlayRate(float PlayRate)
{
	if (UAnimInstance* AnimInst = GetOwnerAnimInstance())
	{
		if (UAnimMontage* AnimMontage = AnimInst->GetCurrentActiveMontage())
		{
			AnimInst->Montage_SetPlayRate(AnimMontage, PlayRate);
		}
	}
}
