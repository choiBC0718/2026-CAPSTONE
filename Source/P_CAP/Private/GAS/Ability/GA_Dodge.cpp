// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GA_Dodge.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Tasks/AbilityTask_RotateToCursor.h"
#include "P_CAP/P_CAP.h"


UGA_Dodge::UGA_Dodge()
{
	DodgeCastTag = UCAP_AbilitySystemStatics::GetItemTriggerCastDodge();
}

void UGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UCAP_WeaponComponent* WeaponComp = GetWeaponComponentFromActorInfo(ActorInfo))
		WeaponComp->ConsumeDodge();

	ACharacter* Character =Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		K2_EndAbility();
		return;
	}
	if (ACAP_PlayerCharacter* Player =GetPlayerCharacterFromActorInfo())
		Player->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Ignore);
	

	UAbilityTask_RotateToCursor* RotToCursor = UAbilityTask_RotateToCursor::SmoothRotateToMouse(this, 2000.f);
	RotToCursor->ReadyForActivation();

	FVector DashDirection = Character->GetActorForwardVector();
	
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(ECC_TargetGround, false, HitResult))
		{
			DashDirection = HitResult.ImpactPoint - Character->GetActorLocation();
			DashDirection.Z = 0.f;
			DashDirection.Normalize();
		}
	}

	UAbilityTask_ApplyRootMotionConstantForce* ForceTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this, NAME_None, DashDirection, DodgeSpeed, DodgeDuration, false, nullptr,
		ERootMotionFinishVelocityMode::SetVelocity, FVector::ZeroVector, 0.f, true);
	ForceTask->ReadyForActivation();

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DodgeMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_Dodge::K2_EndAbility);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Dodge::K2_EndAbility);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Dodge::K2_EndAbility);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Dodge::K2_EndAbility);
	MontageTask->ReadyForActivation();

	BroadcastTriggerEvent(DodgeCastTag);
}

bool UGA_Dodge::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (UCAP_WeaponComponent* WeaponComp = GetWeaponComponentFromActorInfo(ActorInfo))
		return WeaponComp->CanDodge() && Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	return false;
}

void UGA_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ACAP_PlayerCharacter* Player =GetPlayerCharacterFromActorInfo())
		Player->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Block);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

class UCAP_WeaponComponent* UGA_Dodge::GetWeaponComponentFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
		return nullptr;

	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (Player && Player->GetWeaponComponent())
		return Player->GetWeaponComponent();
	
	return nullptr;
}
