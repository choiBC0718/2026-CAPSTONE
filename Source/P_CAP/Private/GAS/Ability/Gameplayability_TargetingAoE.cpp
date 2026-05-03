   // Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Gameplayability_TargetingAoE.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Engine/OverlapResult.h"
#include "GAS/Actors/CAP_ProjectileBase.h"
#include "GAS/Actors/CAP_TargetActor.h"
#include "GAS/Actors/CAP_TargetRangeIndicator.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Tasks/AbilityTask_TickRotToCursor.h"

UGameplayability_TargetingAoE::UGameplayability_TargetingAoE()
{
	BlockAbilitiesWithTag.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());
}

void UGameplayability_TargetingAoE::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                       const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const FWeaponSkillData* SkillData = GetSkillDataFromContext(Handle, ActorInfo);
	if (!SkillData)
		return;
	const FTargetingLogicData* TargetingLogic = SkillData->LogicData.GetPtr<FTargetingLogicData>();
	if (!TargetingLogic)
		return;
	
   	if (TargetingLogic->RangeIndicatorClass)
   	{
   		FVector SpawnLoc = GetAvatarActorFromActorInfo()->GetActorLocation();
   		SpawnedRangeIndicator = GetWorld()->SpawnActor<ACAP_TargetRangeIndicator>(TargetingLogic->RangeIndicatorClass.Get(), SpawnLoc, FRotator::ZeroRotator);
   		if (SpawnedRangeIndicator)
   		{
   			SpawnedRangeIndicator->AttachToActor(GetAvatarActorFromActorInfo(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
   			SpawnedRangeIndicator->Initialize(TargetingLogic->MaxTargetingRange);
   		}
   	}

   	if (TargetingLogic->TargetActorClass)
   	{
   		TargetActorClass = TargetingLogic->TargetActorClass.Get();
   	}

   	UAbilityTask_TickRotToCursor* RotToCursor = UAbilityTask_TickRotToCursor::TickRotToCursor(this, 500.f);
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
   		GroundPick->Initialize(TargetingLogic->MaxTargetingRange, TargetingLogic->TargetAreaRadius);
   	}
   	WaitTargetData->FinishSpawningActor(this, TargetActor);
}

void UGameplayability_TargetingAoE::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	   const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility, bool bWasCancelled)
{
   	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
   	if (SpawnedRangeIndicator)
   		SpawnedRangeIndicator->Destroy();
   	bIsCasting = false;
}

   void UGameplayability_TargetingAoE::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
   	if (SpawnedRangeIndicator)
   		SpawnedRangeIndicator->Destroy();
	
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());   	
   	if (!SkillData)
   	{
   		K2_EndAbility();
   		return;
   	}
	const FTargetingLogicData* TargetingLogic = SkillData->LogicData.GetPtr<FTargetingLogicData>();
	if (!TargetingLogic || !TargetingLogic->CastMontage.Get())
		return;

	bIsCasting = true;
   	if (Data.Data.IsValidIndex(0))
   	{
   		const FHitResult* Hit = Data.Data[0]->GetHitResult();
   		if (Hit)
   			CachedTargetLocation = Hit->ImpactPoint;
   	}
   	
   	// 포물선 투사체 타게팅
   	if (TargetingLogic->ProjectileClass.Get())
   	{
   		ProjectileTargetingConfirmed();
   	}
   	else
   	{
   		InstantTargetingConfirmed(Data);
   	}
	
   	UAbilityTask_PlayMontageAndWait* CastMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, TargetingLogic->CastMontage.Get());
   	CastMontage->OnCancelled.AddDynamic(this, &UGameplayability_TargetingAoE::K2_EndAbility);
   	CastMontage->OnCompleted.AddDynamic(this, &UGameplayability_TargetingAoE::K2_EndAbility);
   	CastMontage->OnBlendOut.AddDynamic(this, &UGameplayability_TargetingAoE::K2_EndAbility);
   	CastMontage->OnInterrupted.AddDynamic(this, &UGameplayability_TargetingAoE::K2_EndAbility);
   	CastMontage->ReadyForActivation();
}

void UGameplayability_TargetingAoE::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
   	K2_EndAbility();
}

void UGameplayability_TargetingAoE::ProjectileTargetingConfirmed()
{
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	if (!SkillData)
		return;
	const FTargetingLogicData* TargetingLogic = SkillData->LogicData.GetPtr<FTargetingLogicData>();
	if (!TargetingLogic)
		return;
	
	FVector SpawnLoc = GetMuzzleSocketLocation(TargetingLogic->ProjectileSocketName);
	if (TargetingLogic->FallingSpawnHeight > 0.f)
	{
		FVector TraceStart = CachedTargetLocation;
		FVector TraceEnd = CachedTargetLocation + FVector(0.f,0.f, TargetingLogic->FallingSpawnHeight);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetAvatarActorFromActorInfo());

		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit,TraceStart,TraceEnd,ECC_Visibility, Params);
		if (bHit)
		{
			SpawnLoc = TraceEnd;
		}
	}
	
	TArray<ACAP_ProjectileBase*> Projectiles = SpawnProjectile(SpawnLoc, TargetingLogic);
	if (Projectiles.Num() > 0)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GetDamageGE(), GetAbilityLevel());
		EffectSpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierDataTag, SkillData->DamageMultiplier);

		FGameplayTag HitTag = IsBasicAttack() ? TriggerHitBasicTag : TriggerHitAbilityTag;
		for (ACAP_ProjectileBase* Projectile : Projectiles)
			Projectile->InitProjectile(CachedTargetLocation,TargetingLogic->TargetAreaRadius,0.5f,EffectSpecHandle, SkillData->GameplayCueTag, HitTag);
	}
}

void UGameplayability_TargetingAoE::InstantTargetingConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	if (!SkillData)
		return;
	const FTargetingLogicData* TargetingLogic = SkillData->LogicData.GetPtr<FTargetingLogicData>();
	if (!TargetingLogic)
		return;
	
	if (Data.Data.IsValidIndex(0))
	{
		const FHitResult* Hit = Data.Data[0]->GetHitResult();
		if (Hit)
		{
			SendGameplayCueEvent(*Hit,SkillData);
			TArray<FOverlapResult> Overlaps;
			FCollisionObjectQueryParams ObjQueryParams;
			ObjQueryParams.AddObjectTypesToQuery(ECC_Pawn);
			FCollisionShape SphereShape = FCollisionShape::MakeSphere(TargetingLogic->TargetAreaRadius);

			bool bHit = GetWorld()->OverlapMultiByObjectType(Overlaps, Hit->ImpactPoint,FQuat::Identity, ObjQueryParams, SphereShape);
			if (bHit)
			{
				FGameplayAbilityTargetDataHandle AggregatedTargetData;
				for (const FOverlapResult& Overlap : Overlaps)
				{
					AActor* OverlapActor = Overlap.GetActor();
					if (OverlapActor && OverlapActor!=GetAvatarActorFromActorInfo())
					{
						FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(OverlapActor);
						BP_ApplyGameplayEffectToTarget(TargetData, GetDamageGE(), GetAbilityLevel());
						AggregatedTargetData.Append(TargetData);
					}
				}
				if (AggregatedTargetData.Num() > 0)
					BroadcastTriggerEvent(IsBasicAttack()?TriggerHitBasicTag:TriggerHitAbilityTag, AggregatedTargetData);
			}
		}
	}
}
