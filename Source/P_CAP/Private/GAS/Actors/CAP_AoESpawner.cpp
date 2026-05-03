// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Actors/CAP_AoESpawner.h"

#include "CAP_ProjectileBase.h"
#include "Components/DecalComponent.h"
#include "Engine/OverlapResult.h"
#include "P_CAP/P_CAP.h"

ACAP_AoESpawner::ACAP_AoESpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

	Decal = CreateDefaultSubobject<UDecalComponent>("Decal");
	Decal->SetupAttachment(Root);
	Decal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
}

void ACAP_AoESpawner::InitializeSpawner(const FAoESpawnerSetupData& InData)
{
	SetupData = InData;
	Decal->DecalSize = FVector(200.f, SetupData.SpawnAreaRadius, SetupData.SpawnAreaRadius);
	if (SetupData.DelayTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ACAP_AoESpawner::OnDelayFinished, SetupData.DelayTime, false);
	}
	else
	{
		OnDelayFinished();
	}
}

void ACAP_AoESpawner::OnDelayFinished()
{
	if (!SetupData.ProjectileClass)
	{
		Destroy();
		return;
	}
	switch (SetupData.SpawnMethod)
	{
		case EAoESpawnMethod::RandomFalling:
			SpawnRandomFalling();
			break;
		case EAoESpawnMethod::TargetedFalling:
			SpawnTargetedFalling();
			break;
		case EAoESpawnMethod::Homing:
			SpawnHoming();
			break;
	}
	Destroy();
}

void ACAP_AoESpawner::SpawnRandomFalling()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i=0 ; i<SetupData.NumProjectiles ; i++)
	{
		FVector2D RandPoint = FMath::RandPointInCircle(SetupData.SpawnAreaRadius);
		FVector SpawnLoc = GetActorLocation() + FVector(RandPoint.X, RandPoint.Y, SetupData.FallingHeight);

		FVector FallDir = FVector(FMath::RandRange(-0.3f,0.3f), FMath::RandRange(-0.3f,0.3f), -1.f).GetSafeNormal();

		ACAP_ProjectileBase* Projectile = GetWorld()->SpawnActor<ACAP_ProjectileBase>(SetupData.ProjectileClass, SpawnLoc, FallDir.Rotation(), SpawnParams);
		if (Projectile)
		{
			Projectile->ProjectileType = EProjectileType::Falling;
			Projectile->InitProjectile(FallDir, SetupData.ExplosionRadius,0.f, SetupData.DamageSpecHandle, SetupData.CueTag, SetupData.TriggerItemProjHitTag, nullptr);
		}
	}
}

void ACAP_AoESpawner::SpawnTargetedFalling()
{
	TArray<USceneComponent*> EnemyRoots = FindEnemyRootComponent();
	if (EnemyRoots.Num() == 0)
		return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	int32 SpawnCount = FMath::Min(SetupData.NumProjectiles, EnemyRoots.Num());

	for (int32 i=0 ; i<SpawnCount ; i++)
	{
		if (!EnemyRoots[i])
			continue;
		FVector SpawnLoc = EnemyRoots[i]->GetComponentLocation() + FVector(0.f,0.f,SetupData.FallingHeight);
		FVector FallDir = FVector(0.f,0.f,-1.f);

		ACAP_ProjectileBase* Projectile = GetWorld()->SpawnActor<ACAP_ProjectileBase>(SetupData.ProjectileClass,SpawnLoc,FRotator::ZeroRotator,SpawnParams);
		if (Projectile)
		{
			Projectile->ProjectileType = EProjectileType::Falling;
			Projectile->InitProjectile(FallDir, SetupData.ExplosionRadius, 0.f, SetupData.DamageSpecHandle, SetupData.CueTag, SetupData.TriggerItemProjHitTag, nullptr);
		}
	}
}

void ACAP_AoESpawner::SpawnHoming()
{
	TArray<USceneComponent*> EnemyRoots = FindEnemyRootComponent();
	
	if (EnemyRoots.Num() == 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 SpawnCount = FMath::Min(SetupData.NumProjectiles, EnemyRoots.Num());

	FVector SpawnLoc = GetActorLocation();
	FVector ForwardDir = GetInstigator()->GetActorForwardVector();
	ForwardDir.Z = 0.f; // 수평 유지를 위해 Z축 제거
	ForwardDir.Normalize();
	
	for (int32 i = 0; i < SpawnCount; i++)
	{
		if (!EnemyRoots[i]) continue;
		
		
		ACAP_ProjectileBase* Projectile = GetWorld()->SpawnActor<ACAP_ProjectileBase>(SetupData.ProjectileClass, SpawnLoc, ForwardDir.Rotation(), SpawnParams);
		if (Projectile)
		{
			Projectile->ProjectileType = EProjectileType::Homing;
			Projectile->InitProjectile(ForwardDir, SetupData.ExplosionRadius, 0.f, SetupData.DamageSpecHandle, SetupData.CueTag, SetupData.TriggerItemProjHitTag, EnemyRoots[i]);
		}
	}
}

TArray<class USceneComponent*> ACAP_AoESpawner::FindEnemyRootComponent()
{
	TArray<USceneComponent*> ValidTargets;
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjQueryParams;
	ObjQueryParams.AddObjectTypesToQuery(ECC_Hitbox);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SetupData.SpawnAreaRadius);
	bool bHit = GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, ObjQueryParams, SphereShape);
	if (bHit)
	{
		for (const FOverlapResult& OverlapResult : Overlaps)
		{
			AActor* Enemy = OverlapResult.GetActor();
			if (Enemy && Enemy!=GetInstigator())
			{
				ValidTargets.Add(Enemy->GetRootComponent());
			}
		}
	}
	return ValidTargets;
}


