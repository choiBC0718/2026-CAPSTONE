   // Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Flow/Gameplayability_TargetingAoE.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GAS/Ability/Payload/GA_PayloadBase.h"
#include "GAS/Actors/CAP_TargetActor.h"
#include "GAS/Actors/CAP_TargetRangeIndicator.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Tasks/AbilityTask_TickRotToCursor.h"

UGameplayability_TargetingAoE::UGameplayability_TargetingAoE()
{
	BlockAbilitiesWithTag.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());
	ConfirmTag = FGameplayTag::RequestGameplayTag("Ability.Event.TargetConfirmed");
	SpawnedRangeIndicator = nullptr;
}

void UGameplayability_TargetingAoE::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                       const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const FWeaponSkillData* SkillData = GetSkillDataFromContext(Handle, ActorInfo);
	if (!SkillData || !TargetActorClass)
	{
		K2_EndAbility();
		return;
	}
	
   	if (RangeIndicatorClass)
   	{
   		FVector SpawnLoc = GetAvatarActorFromActorInfo()->GetActorLocation();
   		SpawnedRangeIndicator = GetWorld()->SpawnActor<ACAP_TargetRangeIndicator>(RangeIndicatorClass.Get(), SpawnLoc, FRotator::ZeroRotator);
   		if (SpawnedRangeIndicator)
   		{
   			SpawnedRangeIndicator->AttachToActor(GetAvatarActorFromActorInfo(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
   			SpawnedRangeIndicator->Initialize(MaxTargetingRange);
   		}
   	}

	float ReticleRadius = 0.f;
	if (SkillData->PayloadAbilityClass.Num() > 0 && SkillData->PayloadAbilityClass[0])
	{
		if (UGA_PayloadBase* PayloadCDO = Cast<UGA_PayloadBase>(SkillData->PayloadAbilityClass[0]->GetDefaultObject()))
		{
			ReticleRadius = PayloadCDO->GetPayloadTargetingRadius();
		}
	}

   	RotToCursor = UAbilityTask_TickRotToCursor::TickRotToCursor(this, TickRotToCursorSpeed);
   	RotToCursor->ReadyForActivation();

	UAbilityTask_WaitTargetData* WaitTargetData = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None,EGameplayTargetingConfirmation::UserConfirmed,TargetActorClass);
	WaitTargetData->ValidData.AddDynamic(this, &UGameplayability_TargetingAoE::TargetConfirmed);
	WaitTargetData->Cancelled.AddDynamic(this, &UGameplayability_TargetingAoE::TargetCancelled);
	WaitTargetData->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor = nullptr;
	WaitTargetData->BeginSpawningActor(this,TargetActorClass,TargetActor);
	ACAP_TargetActor* GroundPick = Cast<ACAP_TargetActor>(TargetActor);
   	if (GroundPick)
   	{
   		GroundPick->Initialize(MaxTargetingRange, ReticleRadius);
   	}
   	WaitTargetData->FinishSpawningActor(this, TargetActor);
}

void UGameplayability_TargetingAoE::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	   const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility, bool bWasCancelled)
{
   	if (SpawnedRangeIndicator)
   		SpawnedRangeIndicator->Destroy();
   	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

   void UGameplayability_TargetingAoE::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
   	if (SpawnedRangeIndicator)
   		SpawnedRangeIndicator->Destroy();
	
	if (RotToCursor)
	{
		RotToCursor->EndTask();
		RotToCursor = nullptr;
	}
	
	if (MontageTask)
		MontageTask->OnInterrupted.RemoveDynamic(this, &UGameplayability_TargetingAoE::K2_EndAbility);
	
	FGameplayEventData EventData;
	// 클릭 좌표 포함
	EventData.TargetData = Data;
	EventData.Instigator = GetAvatarActorFromActorInfo();
	EventData.EventMagnitude = ChargedTime;
	// Payload가 트리거를 받을 태그로 이벤트 보냄
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActorFromActorInfo(), ConfirmTag, EventData);
	
	if (CastMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayCastMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None,CastMontage);
		PlayCastMontage->OnCancelled.AddDynamic(this, &UGameplayability_TargetingAoE::K2_EndAbility);
		PlayCastMontage->OnBlendOut.AddDynamic(this, &UGameplayability_TargetingAoE::K2_EndAbility);
		PlayCastMontage->OnCompleted.AddDynamic(this, &UGameplayability_TargetingAoE::K2_EndAbility);
		PlayCastMontage->OnInterrupted.AddDynamic(this, &UGameplayability_TargetingAoE::K2_EndAbility);
		PlayCastMontage->ReadyForActivation();
	}
}

void UGameplayability_TargetingAoE::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
   	K2_EndAbility();
}