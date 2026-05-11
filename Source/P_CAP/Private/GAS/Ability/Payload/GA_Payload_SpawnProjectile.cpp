// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Payload/GA_Payload_SpawnProjectile.h"

#include "Engine/OverlapResult.h"
#include "GAS/Actors/CAP_ProjectileBase.h"
#include "P_CAP/P_CAP.h"

UGA_Payload_SpawnProjectile::UGA_Payload_SpawnProjectile()
{
}

void UGA_Payload_SpawnProjectile::ExecutePayloadLogic(const FGameplayEventData& EventData)
{
	if (!ProjectileClass)
		return;
	
	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (!OwnerActor)
		return;

	FVector SpawnLoc = OwnerActor->GetActorLocation();
	FRotator SpawnRot = OwnerActor->GetActorRotation();
	FVector TargetLoc = FVector::ZeroVector;

	if (EventData.TargetData.Num()>0)
	{
		if (const FHitResult* Hit = EventData.TargetData.Data[0]->GetHitResult())
		{
			TargetLoc = Hit->ImpactPoint;
			if (ProjectileType == EProjectileType::Falling)
			{
				SpawnLoc=Hit->ImpactPoint + FVector(0.f,0.f,FallingSpawnHeight);
				SpawnRot=FRotator(-90.f,0.f,0.f);
			}
		}
	}
	//UE_LOG(LogTemp,Warning,TEXT("투사체 목표 위치 = %f	  %f  %f"),TargetLoc.X, TargetLoc.Y,TargetLoc.Z);
	
	if (TargetLoc.IsZero())
		TargetLoc = SpawnLoc + SpawnRot.Vector() * MaxDistance;
	if (ProjectileType != EProjectileType::Falling)
		SpawnLoc = GetMuzzleSocketLocation(MuzzleSocketName);
	
	SpawnProjectile(SpawnLoc, SpawnRot,TargetLoc, EventData);
}

void UGA_Payload_SpawnProjectile::SpawnProjectile(FVector SpawnLoc, FRotator SpawnRot, FVector TargetLoc,const FGameplayEventData& EventData)
{
	TSubclassOf<UGameplayEffect> DamageGE = GetDamageGE();
	FGameplayEffectSpecHandle DamageSpecHandle;
	if (DamageGE)
		DamageSpecHandle = MakeOutgoingGameplayEffectSpec(GetDamageGE(), GetAbilityLevel());
	
	const FWeaponSkillData* SkillData = GetSkillDataFromContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	if (DamageSpecHandle.IsValid() && SkillData)
	{
		DamageSpecHandle.Data->SetSetByCallerMagnitude(BaseDamageDataTag, SkillData->BaseDamage);
		DamageSpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierDataTag, SkillData->DamageMultiplier);
		DamageSpecHandle.Data->SetSetByCallerMagnitude(ChargeMultiplierDataTag, EventData.EventMagnitude);
	}

	FGameplayTag HitTag = IsBasicAttack() ? TriggerHitBasicTag : TriggerHitAbilityTag;
	FGameplayTag CueTag = SkillData ? SkillData->GameplayCueTag : FGameplayTag();
	
	AActor* OwnerAvatarActor = GetAvatarActorFromActorInfo();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerAvatarActor;
	SpawnParams.Instigator = Cast<APawn>(OwnerAvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	USceneComponent* TargetComp = nullptr;
	if (ProjectileType == EProjectileType::Homing)
	{
		TargetComp = FindHomingTarget(SpawnLoc);
	}
	
	int32 ProjNums = FMath::Max(1, NumOfProjectiles);
	for (int32 i=0 ; i<ProjNums ; i++)
	{
		float CurrentAngle = 0.f;
		if (ProjNums > 1)
		{
			float HalfAngle = SpreadAngle/ 2.f;
			float Step = SpreadAngle / (ProjNums - 1);
			CurrentAngle = -HalfAngle + (i*Step);
		}
		FRotator FinalSpawnRot = SpawnRot;
		FinalSpawnRot.Yaw += CurrentAngle;
		FTransform SpawnTrans(FinalSpawnRot, SpawnLoc);

		FVector DirToTarget = TargetLoc - SpawnLoc;
		DirToTarget = DirToTarget.RotateAngleAxis(CurrentAngle, FVector::UpVector);
		FVector FinalTargetLoc = SpawnLoc + DirToTarget;
		
		ACAP_ProjectileBase* Projectile = GetWorld()->SpawnActorDeferred<ACAP_ProjectileBase>(ProjectileClass,SpawnTrans, SpawnParams.Owner, SpawnParams.Instigator, SpawnParams.SpawnCollisionHandlingOverride);
		if (Projectile)
		{
			FProjectileInitData InitData;
			InitData.ProjectileType		= ProjectileType;
			InitData.ProjectileSpeed	= ProjectileSpeed;
			InitData.MaxDistance		= MaxDistance;
			InitData.ExplosionRadius	= ExplosionRadius;
			InitData.ArcTension			= ArcTension;
			InitData.MaxHitCount		= MaxHitCount;

			InitData.LaunchDir = (ProjectileType == EProjectileType::Falling) ? FVector::DownVector : FinalSpawnRot.Vector();
			InitData.DamageSpecHandle	= DamageSpecHandle;
			InitData.CueTag				= CueTag;
			InitData.HitTriggerTag		= HitTag;
			InitData.HomingTarget		= TargetComp;
			InitData.TargetLocation		= FinalTargetLoc;

			Projectile->InitProjectile(InitData);
			Projectile->FinishSpawning(SpawnTrans);
		}
	}
}

class USceneComponent* UGA_Payload_SpawnProjectile::FindHomingTarget(const FVector& SearchOrigin)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	USceneComponent* ClosestTarget = nullptr;
	float ClosestDist = MAX_FLT;
	
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjQueryParams;
	ObjQueryParams.AddObjectTypesToQuery(ECC_Hitbox);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(HomingSearchRadius);
	bool bHit = GetWorld()->OverlapMultiByObjectType(Overlaps, SearchOrigin, FQuat::Identity, ObjQueryParams, SphereShape);
	if (bHit)
	{
		for (const FOverlapResult& OverlapResult : Overlaps)
		{
			AActor* Enemy = OverlapResult.GetActor();
			if (Enemy && Enemy!=AvatarActor)
			{
				float Dist = FVector::Distance(SearchOrigin, Enemy->GetActorLocation());
				if (Dist < ClosestDist)
				{
					ClosestDist = Dist;
					ClosestTarget = Enemy->GetRootComponent();
				}
			}
		}
	}
	return ClosestTarget;
}

