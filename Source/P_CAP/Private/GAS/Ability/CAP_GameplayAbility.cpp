// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_GameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Animation/AN_SendRMSEvent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "GAS/Tasks/AbilityTask_RotateToCursor.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Items/Weapon/CAP_WeaponInstance.h"
#include "Kismet/KismetMathLibrary.h"


UCAP_GameplayAbility::UCAP_GameplayAbility()
{
	DamageTag = UCAP_AbilitySystemStatics::GetDamageTag();
	RMSTag = UCAP_AbilitySystemStatics::GetRMSTag();
	ActivationOwnedTags.AddTag(UCAP_AbilitySystemStatics::GetMovementBlockStateTag());
}

void UCAP_GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	const FWeaponSkillData* SkillData = GetCurrentSkillData();
	if (!SkillData)
		return;
	
	if (SkillData->AbilityMontage)
		AbilityMontage = SkillData->AbilityMontage;
	if (SkillData->SkillDamageTypeEffect)
		AbilityDamageEffect = SkillData->SkillDamageTypeEffect;

	// 마우스 방향으로 회전 이벤트
	UAbilityTask_RotateToCursor* RotateTask = UAbilityTask_RotateToCursor::SmoothRotateToMouse(this, 1500.f);
	RotateTask->ReadyForActivation();

	// 스킬 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AbilityMontage);
	MontageTask->OnCancelled.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
	MontageTask->OnCompleted.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
	MontageTask->ReadyForActivation();

	// 데미지 이벤트 처리
	UAbilityTask_WaitGameplayEvent* WaitDamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, DamageTag);
	WaitDamageTask->EventReceived.AddDynamic(this, &UCAP_GameplayAbility::OnDamageTagReceived);
	WaitDamageTask->ReadyForActivation();

	// 가짜 루트 모션 이벤트 처리
	UAbilityTask_WaitGameplayEvent* WaitRMSTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, RMSTag);
	WaitRMSTask->EventReceived.AddDynamic(this, &UCAP_GameplayAbility::OnRMSTagReceived);
	WaitRMSTask->ReadyForActivation();
}

void UCAP_GameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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

void UCAP_GameplayAbility::RotateToMouseCursor()
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!Player)
		return;
	
	if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
	{
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(ECC_Visibility,false,HitResult))
		{
			FVector StartLoc = Player->GetActorLocation();
			FVector TargetLoc = HitResult.ImpactPoint;
			FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(StartLoc, TargetLoc);
			FRotator CurrentRot = Player->GetActorRotation();
			FRotator NewRot = FRotator(CurrentRot.Pitch,LookRotation.Yaw, CurrentRot.Roll);

			Player->SetActorRotation(NewRot);
		}	
	}
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

void UCAP_GameplayAbility::OnDamageTagReceived(FGameplayEventData Payload)
{
	if (AbilityDamageEffect == nullptr)
		return;
	
	int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(Payload.TargetData);
	for (int i =0; i<HitResultCount; i++)
	{
		FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Payload.TargetData, i);
		
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(AbilityDamageEffect, GetAbilityLevel(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()));
		FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
		EffectContext.AddHitResult(HitResult);
		EffectSpecHandle.Data -> SetContext(EffectContext);

		ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
	}
}

void UCAP_GameplayAbility::OnRMSTagReceived(FGameplayEventData Payload)
{
	const UAN_SendRMSEvent* RMSNotify = Cast<UAN_SendRMSEvent>(Payload.OptionalObject);
	if (!RMSNotify)
		return;

	ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo();
	if (Player)
	{
		FVector ForwardDir = Player->GetActorForwardVector();
		UAbilityTask_ApplyRootMotionConstantForce* RMTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this, NAME_None,ForwardDir, RMSNotify->RMSStrength, RMSNotify->RMSDuration,false, nullptr,ERootMotionFinishVelocityMode::SetVelocity,
			FVector::ZeroVector, 0.f, false);
		RMTask->ReadyForActivation();
	}
}

class ACAP_PlayerCharacter* UCAP_GameplayAbility::GetPlayerCharacterFromActorInfo() const
{
	return Cast<ACAP_PlayerCharacter>(GetAvatarActorFromActorInfo());
}

class UCAP_WeaponDataAsset* UCAP_GameplayAbility::GetWeaponDataAsset() const
{
	if (ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo())
	{
		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
		{
			if (UCAP_WeaponInstance* WeaponInst = WeaponComp->GetCurrentWeaponInstance())
			{
				return WeaponInst->GetWeaponDA();
			}
		}
	}
	return nullptr;
}

const struct FWeaponSkillData* UCAP_GameplayAbility::GetCurrentSkillData() const
{
	ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo();
	if (!Player)		return nullptr;

	UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent();
	if (!WeaponComp)	return nullptr;
	
	UCAP_WeaponInstance* WeaponInst = WeaponComp->GetCurrentWeaponInstance();
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
