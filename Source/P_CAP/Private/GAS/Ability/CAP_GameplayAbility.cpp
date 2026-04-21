// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_GameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Animation/AN_SendRMSEvent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "GAS/Actors/CAP_ProjectileBase.h"
#include "GAS/Tasks/AbilityTask_RotateToCursor.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Items/Weapon/CAP_WeaponInstance.h"


UCAP_GameplayAbility::UCAP_GameplayAbility()
{
	DamageTag = UCAP_AbilitySystemStatics::GetDamageTag();
	RMSTag = UCAP_AbilitySystemStatics::GetRMSTag();
	SpawnProjectileTag = UCAP_AbilitySystemStatics::GetSpawnProjectileTag();
	
	ActivationOwnedTags.AddTag(UCAP_AbilitySystemStatics::GetMovementBlockStateTag());
}

void UCAP_GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	SkillData = GetCurrentSkillData();
	if (!SkillData || !SkillData->AbilityMontage.Get() || !SkillData->SkillDamageTypeEffect.Get())
	{
		K2_EndAbility();
		return;
	}
	
	bIsCasting = false;
	AbilityMontage = SkillData->AbilityMontage.Get();
	AbilityDamageEffect = SkillData->SkillDamageTypeEffect.Get();

	// 마우스 방향으로 회전 이벤트
	UAbilityTask_RotateToCursor* RotateTask = UAbilityTask_RotateToCursor::SmoothRotateToMouse(this, 1500.f);
	RotateTask->ReadyForActivation();

	// 스킬 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AbilityMontage);
	MontageTask->OnCancelled.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
	MontageTask->OnCompleted.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UCAP_GameplayAbility::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UCAP_GameplayAbility::OnMontageInterrupted);
	MontageTask->ReadyForActivation();
	
	// 가짜 루트 모션 이벤트 처리
	UAbilityTask_WaitGameplayEvent* WaitRMSTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, RMSTag);
	WaitRMSTask->EventReceived.AddDynamic(this, &UCAP_GameplayAbility::OnRMSTagReceived);
	WaitRMSTask->ReadyForActivation();
	
	if (SkillData->LogicType == ESkillLogicType::Melee)
	{
		// 데미지 이벤트 처리
		UAbilityTask_WaitGameplayEvent* WaitDamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, DamageTag);
		WaitDamageTask->EventReceived.AddDynamic(this, &UCAP_GameplayAbility::OnDamageTagReceived);
		WaitDamageTask->ReadyForActivation();
	}
	else if (SkillData->LogicType == ESkillLogicType::Projectile)
	{
		UAbilityTask_WaitGameplayEvent* SpawnProjectileTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, SpawnProjectileTag,nullptr,false,false);
		SpawnProjectileTask->EventReceived.AddDynamic(this, &UCAP_GameplayAbility::OnSpawnProjectileTagReceived);
		SpawnProjectileTask->ReadyForActivation();
	}
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

		SendGameplayCueEvent(HitResult);
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

void UCAP_GameplayAbility::OnSpawnProjectileTagReceived(FGameplayEventData Payload)
{
	if (!SkillData || !SkillData->ProjectileClass.Get())
		return;
	
	TArray<FName> OutNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(Payload.EventTag, OutNames);

	FVector SocketLoc = GetMuzzleSocketLocation(OutNames.Last());

	TArray<ACAP_ProjectileBase*> Projectiles = SpawnProjectile(SocketLoc);
	
	if (Projectiles.Num() > 0)
	{
		FGameplayEffectSpecHandle EffectSpecHandle;
		if (SkillData->SkillDamageTypeEffect.Get())
		{
			EffectSpecHandle = MakeOutgoingGameplayEffectSpec(SkillData->SkillDamageTypeEffect.Get(), GetAbilityLevel());
		}
		for (ACAP_ProjectileBase* Projectile : Projectiles)
		{
			FVector LaunchDir = Projectile->GetActorForwardVector();
			Projectile->InitStraightProjectile(LaunchDir, EffectSpecHandle,SkillData->GameplayCueTag);
		}
	}
}

void UCAP_GameplayAbility::OnMontageInterrupted()
{
	if (!bIsCasting)
		K2_EndAbility();
}

class ACAP_PlayerCharacter* UCAP_GameplayAbility::GetPlayerCharacterFromActorInfo() const
{
	return Cast<ACAP_PlayerCharacter>(GetAvatarActorFromActorInfo());
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

TArray<class ACAP_ProjectileBase*> UCAP_GameplayAbility::SpawnProjectile(FVector SpawnLoc)
{
	TArray<ACAP_ProjectileBase*> SpawnedProjectiles;
	
	AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerAvatarActor;
	SpawnParams.Instigator = Cast<APawn>(OwnerAvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 ProjNums = FMath::Max(1, SkillData->NumOfProjectiles);
	FRotator BaseRot = OwnerAvatarActor->GetActorRotation();
	
	for (int32 i=0 ; i<ProjNums ; i++)
	{
		float CurrentAngle = 0.f;
		if (ProjNums > 1)
		{
			float HalfAngle = SkillData->SpreadAngle / 2.f;
			float Step = SkillData->SpreadAngle / (ProjNums - 1);
			CurrentAngle = -HalfAngle + (i*Step);
		}
		FRotator SpawnRot = BaseRot;
		SpawnRot.Yaw += CurrentAngle;

		ACAP_ProjectileBase* Projectile = GetWorld()->SpawnActor<ACAP_ProjectileBase>(SkillData->ProjectileClass.Get(),SpawnLoc,SpawnRot, SpawnParams);
		if (Projectile)
			SpawnedProjectiles.Add(Projectile);
	}
	
	return SpawnedProjectiles;
}

FVector UCAP_GameplayAbility::GetMuzzleSocketLocation(FName SocketName)
{
	FVector SocketLoc = GetAvatarActorFromActorInfo()->GetActorLocation();
	ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo();
	
	if (Player)
	{
		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
		{
			USkeletalMeshComponent* R_Weapon = WeaponComp->GetWeaponMesh(EEquipHand::Right);
			USkeletalMeshComponent* L_Weapon = WeaponComp->GetWeaponMesh(EEquipHand::Left);

			if (R_Weapon && R_Weapon->DoesSocketExist(SocketName))
				SocketLoc = R_Weapon->GetSocketLocation(SocketName);
			else if (L_Weapon && L_Weapon->DoesSocketExist(SocketName))
				SocketLoc = L_Weapon->GetSocketLocation(SocketName);
			else
			{
				if (R_Weapon)		SocketLoc = R_Weapon->GetComponentLocation();
				else if (L_Weapon)	SocketLoc = L_Weapon->GetComponentLocation();
			}
		}
	}
	return SocketLoc;
}

void UCAP_GameplayAbility::SendGameplayCueEvent(FHitResult HitResult)
{
	if (SkillData && SkillData->GameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;
		CurrentActorInfo->AbilitySystemComponent->ExecuteGameplayCue(SkillData->GameplayCueTag, CueParams);
	}
}

const struct FWeaponSkillData* UCAP_GameplayAbility::GetCurrentSkillData() const
{
	ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo();
	if (!Player)		return nullptr;

	UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent();
	if (!WeaponComp)	return nullptr;
	
	UCAP_WeaponInstance* WeaponInst = WeaponComp->GetCurrentWeaponInstance();
	if (!WeaponInst)	return nullptr;
	
	UCAP_WeaponDataAsset* WeaponDA = WeaponInst->GetWeaponDA();
	if (!WeaponDA)		return nullptr;

	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (!Spec)			return nullptr;

	// 평타인 경우 DA의 BasicAbility 반환
	if (Spec->InputID == static_cast<int32>(EAbilityInputID::BasicAttack))
	{
		if (const FWeaponSkillData* BasicAttack = WeaponInst->GetBasicAttack())
		{
			return BasicAttack;
		} 
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
