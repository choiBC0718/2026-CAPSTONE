// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_GameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_WeaponComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"


UCAP_GameplayAbility::UCAP_GameplayAbility()
{
	ActivationOwnedTags.AddTag(UCAP_AbilitySystemStatics::GetMovementBlockStateTag());
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

bool UCAP_GameplayAbility::IsBasicAttack() const
{
	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (!Spec)
		return false;

	int32 TargetInputID = Spec->InputID;
	if (TargetInputID >= 100)
		TargetInputID -= 100;

	return TargetInputID == static_cast<int32>(EAbilityInputID::BasicAttack);
}

void UCAP_GameplayAbility::BroadcastTriggerEvent(FGameplayTag EventTag,	FGameplayAbilityTargetDataHandle TargetData, float EventMagnitude) const
{
	if (!EventTag.IsValid()) return;

	FGameplayEventData Payload;
	Payload.Instigator = GetAvatarActorFromActorInfo();
	Payload.TargetData = TargetData;
	Payload.EventMagnitude = EventMagnitude;
	//UE_LOG(LogTemp,Warning,TEXT("트리거 발생 태그 = %s"),*EventTag.ToString());
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActorFromActorInfo(), EventTag, Payload);
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
	if (!InSkillData || !InSkillData->GameplayCueTag.IsValid())
		return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitResult.GetActor());
	
	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal;

	CueParams.Instigator = GetAvatarActorFromActorInfo();
	CueParams.EffectCauser = GetAvatarActorFromActorInfo();
	CueParams.TargetAttachComponent = HitResult.GetComponent();

	if (TargetASC)
	{
		TargetASC->ExecuteGameplayCue(InSkillData->GameplayCueTag, CueParams);
	}
	else if (CurrentActorInfo && CurrentActorInfo->AbilitySystemComponent.IsValid())
		CurrentActorInfo->AbilitySystemComponent->ExecuteGameplayCue(InSkillData->GameplayCueTag, CueParams);
}

void UCAP_GameplayAbility::ApplyStatusEffectsToTarget(AActor* TargetActor, const TArray<EStatusEffectType>& Effects)
{
	if (!TargetActor || Effects.IsEmpty())
		return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
		return;

	for (const EStatusEffectType& EffectType : Effects)
	{
		if (EffectType == EStatusEffectType::None)
			continue;

		TSubclassOf<UGameplayEffect> EffectToApply = nullptr;
		if (EffectToApply)
		{
			FGameplayEffectContextHandle Context = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
			Context.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

			FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(EffectToApply, GetAbilityLevel(), Context);
			if (SpecHandle.IsValid())
			{
				GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
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

	int32 TargetInputID = Spec->InputID;
	if (TargetInputID >= 100)
		TargetInputID -= 100;
	
	if (TargetInputID == static_cast<int32>(EAbilityInputID::BasicAttack))
	{
		return WeaponInst->GetBasicAttack();
	}
	else if (TargetInputID >= static_cast<int32>(EAbilityInputID::Skill1))
	{
		int32 SkillIndex = TargetInputID - static_cast<int32>(EAbilityInputID::Skill1);
		if (WeaponInst->GetGrantedSkills().IsValidIndex(SkillIndex))
		{
			return &WeaponInst->GetGrantedSkills()[SkillIndex];
		}
	}
	return nullptr;
}
