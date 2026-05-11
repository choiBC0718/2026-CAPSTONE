// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Flow/GameplayAbility_Targeting_Jump.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "GAS/Actors/CAP_TargetActor.h"
#include "GAS/Actors/CAP_TargetRangeIndicator.h"
#include "GAS/Tasks/AbilityTask_TickRotToCursor.h"

UGameplayAbility_Targeting_Jump::UGameplayAbility_Targeting_Jump()
{
}

void UGameplayAbility_Targeting_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

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

	RotToCursor = UAbilityTask_TickRotToCursor::TickRotToCursor(this, TickRotToCursorSpeed);
	RotToCursor->ReadyForActivation();

	UAbilityTask_WaitTargetData* WaitTargetData = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None,EGameplayTargetingConfirmation::UserConfirmed,TargetActorClass);
	WaitTargetData->ValidData.AddDynamic(this, &UGameplayAbility_Targeting_Jump::TargetConfirmed);
	WaitTargetData->Cancelled.AddDynamic(this, &UGameplayAbility_Targeting_Jump::TargetCancelled);
	WaitTargetData->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor = nullptr;
	WaitTargetData->BeginSpawningActor(this,TargetActorClass,TargetActor);
	ACAP_TargetActor* GroundPick = Cast<ACAP_TargetActor>(TargetActor);
	if (GroundPick)
	{
		GroundPick->Initialize(MaxTargetingRange, DamageRange);
	}
	WaitTargetData->FinishSpawningActor(this, TargetActor);
}

void UGameplayAbility_Targeting_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (SpawnedRangeIndicator)
		SpawnedRangeIndicator->Destroy();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Targeting_Jump::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	if (SpawnedRangeIndicator)
		SpawnedRangeIndicator->Destroy();
	
	if (RotToCursor)
	{
		RotToCursor->EndTask();
		RotToCursor = nullptr;
	}
	
	if (MontageTask)
		MontageTask->OnInterrupted.RemoveDynamic(this, &UGameplayAbility_Targeting_Jump::K2_EndAbility);

	if (Data.Data.Num() > 0 && Data.Data[0]->GetHitResult())
	{
		CachedTargetLocation = Data.Data[0]->GetHitResult()->ImpactPoint;
	}
	
	FGameplayTag JumpTag = FGameplayTag::RequestGameplayTag("Ability.Event.Jump");
	UAbilityTask_WaitGameplayEvent* WaitJumpTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, JumpTag);
	WaitJumpTask->EventReceived.AddDynamic(this, &UGameplayAbility_Targeting_Jump::OnJumpTagReceived);
	WaitJumpTask->ReadyForActivation();
	
	if (CastMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayCastMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None,CastMontage);
		PlayCastMontage->OnCancelled.AddDynamic(this, &UGameplayAbility_Targeting_Jump::K2_EndAbility);
		PlayCastMontage->OnBlendOut.AddDynamic(this, &UGameplayAbility_Targeting_Jump::K2_EndAbility);
		PlayCastMontage->OnCompleted.AddDynamic(this, &UGameplayAbility_Targeting_Jump::K2_EndAbility);
		PlayCastMontage->OnInterrupted.AddDynamic(this, &UGameplayAbility_Targeting_Jump::K2_EndAbility);
		PlayCastMontage->ReadyForActivation();
	}
}

void UGameplayAbility_Targeting_Jump::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	K2_EndAbility();
}

void UGameplayAbility_Targeting_Jump::OnJumpTagReceived(FGameplayEventData Payload)
{
	if (ACAP_PlayerCharacter* Player = GetPlayerCharacterFromActorInfo())
	{
		float CurrentZ = Player->GetActorLocation().Z;
		FVector WarpLocation = FVector(CachedTargetLocation.X, CachedTargetLocation.Y, CurrentZ);
		FRotator LookRot = (WarpLocation - Player->GetActorLocation()).Rotation();
		LookRot.Pitch = 0.f;
		LookRot.Roll = 0.f;
		Player->SetActorRotation(LookRot);

		Player->SetActorLocation(WarpLocation);
	}
}
