// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Payload/GA_Paylaod_SpawnTickDamageActor.h"

#include "GAS/Actors/CAP_OverlapDamageActorBase.h"

void UGA_Paylaod_SpawnTickDamageActor::ExecutePayloadLogic(const FGameplayEventData& EventData)
{
	if (!OverlapDamageActorClass)
		return;

	AActor* OwnerAvatar = GetAvatarActorFromActorInfo();
	if (!OwnerAvatar)
		return;

	TSubclassOf<UGameplayEffect> DamageGE = GetDamageGE();
	FGameplayEffectSpecHandle SpecHandle;
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());

	if (DamageGE && SkillData)
	{
		SpecHandle = MakeOutgoingGameplayEffectSpec(DamageGE, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(BaseDamageDataTag, SkillData->BaseDamage);
			SpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierDataTag, SkillData->DamageMultiplier);
			SpecHandle.Data->SetSetByCallerMagnitude(ChargeMultiplierDataTag, EventData.EventMagnitude);
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerAvatar;
	SpawnParams.Instigator = Cast<APawn>(OwnerAvatar);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	FTransform SpawnTrans(FRotator::ZeroRotator, OwnerAvatar->GetActorLocation());
	ACAP_OverlapDamageActorBase* SpawnedActor = GetWorld()->SpawnActorDeferred<ACAP_OverlapDamageActorBase>(OverlapDamageActorClass,SpawnTrans,OwnerAvatar,SpawnParams.Instigator, SpawnParams.SpawnCollisionHandlingOverride);
	if (SpawnedActor)
	{
		FGameplayTag CueTag = SkillData ? SkillData->GameplayCueTag : FGameplayTag();

		FOverlapDamageActorInitData InitData;
		InitData.DamageSpecHandle = SpecHandle;
		InitData.LifeSpan = LifeSpan;
		InitData.DamageTickRate = DamageTickRate;
		InitData.CollisionRadius = CollisionRadius;
		InitData.CueTag = CueTag;
		
		SpawnedActor->InitSkillActor(InitData);

		FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
		SpawnedActor->AttachToActor(OwnerAvatar, AttachRules);
		SpawnedActor->FinishSpawning(SpawnTrans);
	}
}
