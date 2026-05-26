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
	switch (SetupData.ProjectileType)
	{
		case EProjectileType::Straight:
			SpawnStraight();
			break;
		case EProjectileType::Falling:
			if (SetupData.bRandomLocationInArea)
				SpawnRandomly();
			else
				SpawnOnEnemies();
			break;
		case EProjectileType::Homing:
			SpawnOnEnemies();
			break;
		default:
			break;
	}
	Destroy();
}

void ACAP_AoESpawner::SpawnOnEnemies()
{
	TArray<USceneComponent*> EnemyRoots = FindEnemyRootComponent();
	if (EnemyRoots.Num() == 0)
	{
		SpawnRandomly();
		return;
	}

	int32 SpawnCount = FMath::Min(SetupData.NumProjectiles, EnemyRoots.Num());
	FVector ForwardDir = GetInstigator() ? GetInstigator()->GetActorForwardVector() : GetActorForwardVector();

	for (int32 i = 0; i < SpawnCount; i++)
	{
		if (!EnemyRoots[i]) continue;

		FVector SpawnLoc;
		FVector LaunchDir;
		USceneComponent* HomingTarget = nullptr;

		if (SetupData.ProjectileType == EProjectileType::Homing)
		{
			SpawnLoc = GetActorLocation();
			LaunchDir = ForwardDir;
			HomingTarget = EnemyRoots[i];
		}
		else // Falling (Targeted)
		{
			SpawnLoc = EnemyRoots[i]->GetComponentLocation() + FVector(0.f, 0.f, SetupData.FallingHeight);
			LaunchDir = FVector(0.f, 0.f, -1.f);
		}

		SpawnProjectileDeferred(SpawnLoc, LaunchDir, HomingTarget);
	}
}

void ACAP_AoESpawner::SpawnRandomly()
{
for (int32 i = 0; i < SetupData.NumProjectiles; i++)
	{
		FVector2D RandPoint = FMath::RandPointInCircle(SetupData.SpawnAreaRadius);
		FVector SpawnLoc = GetActorLocation() + FVector(RandPoint.X, RandPoint.Y, SetupData.FallingHeight);
		FVector FallDir = FVector(FMath::RandRange(-0.1f, 0.1f), FMath::RandRange(-0.1f, 0.1f), -1.f).GetSafeNormal();

		SpawnProjectileDeferred(SpawnLoc, FallDir);
	}
}

void ACAP_AoESpawner::SpawnStraight()
{
	FVector SpawnLoc = GetActorLocation();

	FVector BaseDir = GetInstigator() ? GetInstigator()->GetActorForwardVector() : GetActorForwardVector();
	BaseDir.Z = 0.f;
	BaseDir.Normalize();

	int32 ProjNums = FMath::Max(1, SetupData.NumProjectiles);
	for (int32 i = 0; i < ProjNums; i++)
	{
		FVector LaunchDir = BaseDir;
		if (ProjNums > 1 && SetupData.SpreadAngle > 0.f)
		{
			float HalfAngle = SetupData.SpreadAngle / 2.f;
			float Step = SetupData.SpreadAngle / (ProjNums - 1);
			float CurrentAngle = -HalfAngle + (i * Step);
			LaunchDir = BaseDir.RotateAngleAxis(CurrentAngle, FVector::UpVector);
		}
		SpawnProjectileDeferred(SpawnLoc, LaunchDir);
	}
}

void ACAP_AoESpawner::SpawnProjectileDeferred(const FVector& SpawnLoc, const FVector& LaunchDir, USceneComponent* HomingTarget)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTransform SpawnTransform(LaunchDir.Rotation(), SpawnLoc);

	ACAP_ProjectileBase* Projectile = GetWorld()->SpawnActorDeferred<ACAP_ProjectileBase>(
		SetupData.ProjectileClass, SpawnTransform, GetOwner(), GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Projectile)
	{
		FProjectileInitData InitData;
		InitData.ProjectileType		= SetupData.ProjectileType;
		InitData.ProjectileSpeed	= SetupData.ProjectileSpeed;
		InitData.MaxDistance		= SetupData.MaxDistance;
		InitData.ExplosionRadius	= SetupData.ExplosionRadius;
		InitData.MaxHitCount		= SetupData.MaxHitCount;
		InitData.LaunchDir			= LaunchDir;
		InitData.DamageSpecHandle	= SetupData.DamageSpecHandle;
		InitData.CueTag				= SetupData.CueTag;
		InitData.HitTriggerTag		= SetupData.TriggerItemProjHitTag;
		InitData.HomingTarget		= HomingTarget;

		Projectile->InitProjectile(InitData);
		Projectile->FinishSpawning(SpawnTransform);
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


