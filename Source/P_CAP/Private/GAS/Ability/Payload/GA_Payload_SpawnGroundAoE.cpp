// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Payload/GA_Payload_SpawnGroundAoE.h"

#include "GAS/Actors/CAP_OverlapDamageActorBase.h"

void UGA_Payload_SpawnGroundAoE::ExecutePayloadLogic(const FGameplayEventData& EventData)
{
	if (!AoEActorClass)
		return;

	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (!OwnerActor)
		return;

	FVector SpawnLoc = OwnerActor->GetActorLocation();
	FRotator SPawnRot = FRotator::ZeroRotator;

	if (EventData.TargetData.Num() >0)
	{
		if (const FHitResult* Hit = EventData.TargetData.Data[0]->GetHitResult())
			SpawnLoc = Hit->ImpactPoint;
	}

	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(GetDamageGE(), GetAbilityLevel());
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	FGameplayTag CueTag = FGameplayTag();

	if (DamageSpecHandle.IsValid() && SkillData)
	{
		DamageSpecHandle.Data->SetSetByCallerMagnitude(BaseDamageDataTag, SkillData->BaseDamage);
		DamageSpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierDataTag, SkillData->DamageMultiplier);
		CueTag = SkillData->GameplayCueTag;
	}

	FTransform SpawnTrans(SPawnRot, SpawnLoc);
	ACAP_OverlapDamageActorBase* SpawnedAoE = GetWorld()->SpawnActorDeferred<ACAP_OverlapDamageActorBase>(AoEActorClass,SpawnTrans, OwnerActor, Cast<APawn>(OwnerActor), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (SpawnedAoE)
	{
		FOverlapDamageActorInitData InitData;
		InitData.DamageSpecHandle = DamageSpecHandle;
		InitData.LifeSpan = LifeSpan;
		InitData.DamageTickRate = DamageTickRate;
		InitData.CollisionRadius = AoERadius;
		InitData.CueTag = CueTag;

		SpawnedAoE->InitSkillActor(InitData);
		SpawnedAoE->FinishSpawning(SpawnTrans);
	}
}
