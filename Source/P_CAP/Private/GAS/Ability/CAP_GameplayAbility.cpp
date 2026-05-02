// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_GameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Animation/AN_SendBasicTagEvent.h"
#include "Animation/AN_SendRMSEvent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Actors/CAP_ProjectileBase.h"
#include "GAS/Tasks/AbilityTask_RotateToCursor.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Items/Weapon/CAP_WeaponInstance.h"


UCAP_GameplayAbility::UCAP_GameplayAbility()
{
	DamageTag = UCAP_AbilitySystemStatics::GetDamageTag();
	RMSTag = UCAP_AbilitySystemStatics::GetRMSTag();
	SpawnProjectileTag = UCAP_AbilitySystemStatics::GetSpawnProjectileTag();

	BaseDamageDataTag = UCAP_AbilitySystemStatics::GetDataDamageBaseTag();
	DamageMultiplierDataTag = UCAP_AbilitySystemStatics::GetDataDamageMultiplierTag();
	ChargeMultiplierDataTag = UCAP_AbilitySystemStatics::GetAbilityChargeTimeTag();
	DataCooldownTag = UCAP_AbilitySystemStatics::GetDataCooldownTag();
	
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
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(Handle, ActorInfo);
	if (!SkillData || !SkillData->AbilityMontage.Get())
	{
		K2_EndAbility();
		return;
	}
	
	bIsCasting = false;
	ChargedTime = 1.f;
	
	float AttackSpeed = 1.f;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		AttackSpeed = ASC->GetNumericAttribute(UCAP_AttributeSet::GetAttackSpeedAttribute());
	}

	// 마우스 방향으로 회전 이벤트
	UAbilityTask_RotateToCursor* RotateTask = UAbilityTask_RotateToCursor::SmoothRotateToMouse(this, 1500.f);
	RotateTask->ReadyForActivation();

	// 스킬 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillData->AbilityMontage.Get(),AttackSpeed);
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

	SendItemTriggerEvent(false);
}

void UCAP_GameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
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

bool UCAP_GameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
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

void UCAP_GameplayAbility::OnDamageTagReceived(FGameplayEventData Payload)
{
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	if (!SkillData)
		return;
	
	int HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(Payload.TargetData);
	FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(GetDamageGE(), GetAbilityLevel(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo()));

	DamageSpecHandle.Data->SetSetByCallerMagnitude(BaseDamageDataTag, SkillData->BaseDamage);
	DamageSpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierDataTag, SkillData->DamageMultiplier);
	DamageSpecHandle.Data->SetSetByCallerMagnitude(ChargeMultiplierDataTag, ChargedTime);

	for (int i =0; i<HitResultCount; i++)
	{
		FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Payload.TargetData, i);
		
		EffectContext.AddHitResult(HitResult);
		DamageSpecHandle.Data -> SetContext(EffectContext);

		ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, DamageSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
		SendGameplayCueEvent(HitResult, SkillData);
	}

	if (HitResultCount > 0)
	{
		SendItemTriggerEvent(true, Payload.TargetData);
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
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	if (!SkillData || !SkillData->ProjectileClass.Get())
		return;

	const UAN_SendBasicTagEvent* EventNotify = Cast<UAN_SendBasicTagEvent>(Payload.OptionalObject);
	FVector SocketLoc = FVector();
	if (EventNotify && EventNotify->ProjectileMuzzleName!=NAME_None)
	{
		SocketLoc = GetMuzzleSocketLocation(EventNotify->ProjectileMuzzleName);
	}

	TArray<ACAP_ProjectileBase*> Projectiles = SpawnProjectile(SocketLoc, SkillData);
	if (Projectiles.Num() > 0)
	{
		FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(GetDamageGE(), GetAbilityLevel());
		DamageSpecHandle.Data->SetSetByCallerMagnitude(BaseDamageDataTag, SkillData->BaseDamage);
		DamageSpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierDataTag, SkillData->DamageMultiplier);
		DamageSpecHandle.Data->SetSetByCallerMagnitude(ChargeMultiplierDataTag, ChargedTime);
		for (ACAP_ProjectileBase* Projectile : Projectiles)
		{
			FVector LaunchDir = Projectile->GetActorForwardVector();
			Projectile->InitStraightProjectile(LaunchDir, DamageSpecHandle,SkillData->GameplayCueTag, IsBasicAttack());
		}
	}
}

void UCAP_GameplayAbility::OnMontageInterrupted()
{
	if (!bIsCasting)
		K2_EndAbility();
}

TSubclassOf<UGameplayEffect> UCAP_GameplayAbility::GetDamageGE() const
{
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	if (!SkillData)
		return nullptr;
	
	ESkillDamageType DamageType = SkillData->DamageType;
	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!ASC || !ASC->GetGenerics()->GetInstantDamageGE(DamageType))
		return nullptr;
	
	return ASC->GetGenerics()->GetInstantDamageGE(DamageType);
}

void UCAP_GameplayAbility::SendItemTriggerEvent(bool bIsHit, FGameplayAbilityTargetDataHandle TargetData)
{
	FString TagString = TEXT("Item.Trigger.");
	TagString += bIsHit ? TEXT("Hit.") : TEXT("Cast.");
	TagString += IsBasicAttack() ? TEXT("Basic") : TEXT("Ability");
//	UE_LOG(LogTemp,Warning,TEXT("Trigger Tag == %s") , *TagString);
	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName(*TagString));
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = GetAvatarActorFromActorInfo();
	Payload.TargetData = TargetData;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActorFromActorInfo(), EventTag, Payload);
}

bool UCAP_GameplayAbility::IsBasicAttack() const
{
	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	return Spec && Spec->InputID == static_cast<int32>(EAbilityInputID::BasicAttack);
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

TArray<class ACAP_ProjectileBase*> UCAP_GameplayAbility::SpawnProjectile(FVector SpawnLoc, const struct FWeaponSkillData* InSkillData)
{
	TArray<ACAP_ProjectileBase*> SpawnedProjectiles;
	if (!InSkillData)
		return SpawnedProjectiles;
	
	AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerAvatarActor;
	SpawnParams.Instigator = Cast<APawn>(OwnerAvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 ProjNums = FMath::Max(1, InSkillData->NumOfProjectiles);
	FRotator BaseRot = OwnerAvatarActor->GetActorRotation();
	
	for (int32 i=0 ; i<ProjNums ; i++)
	{
		float CurrentAngle = 0.f;
		if (ProjNums > 1)
		{
			float HalfAngle = InSkillData->SpreadAngle / 2.f;
			float Step = InSkillData->SpreadAngle / (ProjNums - 1);
			CurrentAngle = -HalfAngle + (i*Step);
		}
		FRotator SpawnRot = BaseRot;
		SpawnRot.Yaw += CurrentAngle;

		ACAP_ProjectileBase* Projectile = GetWorld()->SpawnActor<ACAP_ProjectileBase>(InSkillData->ProjectileClass.Get(),SpawnLoc,SpawnRot, SpawnParams);
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

void UCAP_GameplayAbility::SendGameplayCueEvent(FHitResult HitResult, const struct FWeaponSkillData* InSkillData)
{
	if (InSkillData && InSkillData->GameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;
		CurrentActorInfo->AbilitySystemComponent->ExecuteGameplayCue(InSkillData->GameplayCueTag, CueParams);
	}
}

const struct FWeaponSkillData* UCAP_GameplayAbility::GetSkillDataFromContext(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid() || !ActorInfo->AbilitySystemComponent.IsValid()) return nullptr;

	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!Player) return nullptr;

	UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent();
	if (!WeaponComp) return nullptr;
	
	UCAP_WeaponInstance* WeaponInst = WeaponComp->GetCurrentWeaponInstance();
	if (!WeaponInst) return nullptr;
	
	FGameplayAbilitySpec* Spec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!Spec) return nullptr;

	if (Spec->InputID == static_cast<int32>(EAbilityInputID::BasicAttack))
	{
		return WeaponInst->GetBasicAttack();
	}
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