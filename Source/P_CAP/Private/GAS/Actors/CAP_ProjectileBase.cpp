// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Actors/CAP_ProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "P_CAP/P_CAP.h"


ACAP_ProjectileBase::ACAP_ProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>("Collision Comp");
	SetRootComponent(CollisionComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("Mesh Component");
	MeshComp->SetupAttachment(CollisionComp);
	
	ProjMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement Component");
	ProjMovementComp->UpdatedComponent = CollisionComp;
	ProjMovementComp->bRotationFollowsVelocity = true;
	ProjMovementComp->bIsHomingProjectile = false;
	ProjMovementComp->bInitialVelocityInLocalSpace = false;
	ProjMovementComp->ProjectileGravityScale = 1.f;

	TrailParticleComp = CreateDefaultSubobject<UParticleSystemComponent>("Trail Particle Comp");
	TrailParticleComp->SetupAttachment(MeshComp);
	
	InitialLifeSpan = 5.f;
}

void ACAP_ProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ACAP_ProjectileBase::OnProjectileOverlap);

	CurrentHitCount = 0;
	HitActors.Empty();
}

void ACAP_ProjectileBase::InitProjectile(const FProjectileInitData& InitData)
{
	ProjectileType		= InitData.ProjectileType;
	ProjectileSpeed		= InitData.ProjectileSpeed;
	MaxDistance			= InitData.MaxDistance;
	ExplosionRadius		= InitData.ExplosionRadius;
	HitEffectHandle		= InitData.DamageSpecHandle;
	HitGameplayCueTag	= InitData.CueTag;
	HitTriggerTag		= InitData.HitTriggerTag;
	MaxHitCount			= InitData.MaxHitCount;

	FVector LaunchDir	= InitData.LaunchDir;
	ProjMovementComp->InitialSpeed = ProjectileSpeed;
	ProjMovementComp->MaxSpeed = ProjectileSpeed;

	if (CollisionComp)
	{
		uint8 TeamIdVal = 255;
		if (IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(GetInstigator()))
		{
			TeamIdVal = TeamAgent->GetGenericTeamId().GetId();
		}

		FName ProfileName;
		if (ProjectileType == EProjectileType::Falling || ProjectileType == EProjectileType::Arc)
			ProfileName = (TeamIdVal == 0) ? FName("PlayerProj_AoE") : FName("EnemyProj_AoE");
		else
			ProfileName = (TeamIdVal == 0) ? FName("PlayerProj_Direct") : FName("EnemyProj_Direct");
		
		CollisionComp->SetCollisionProfileName(ProfileName);
	}
	
	switch (ProjectileType)
	{
		case EProjectileType::Straight:
		{
			ProjMovementComp->Velocity = InitData.LaunchDir.GetSafeNormal() * ProjectileSpeed;
			StraightTypeInit();
			break;
		}
		case EProjectileType::Arc:
		{
			FVector LaunchVel;
			bool bSuccess = UGameplayStatics::SuggestProjectileVelocity_CustomArc(this, LaunchVel, GetActorLocation(), InitData.TargetLocation, 0.f, InitData.ArcTension);
			if (bSuccess)
			{
				ProjMovementComp->InitialSpeed = 0.f;
				ProjMovementComp->MaxSpeed = 0.f;
				ProjMovementComp->Velocity = LaunchVel;
				ProjMovementComp->ProjectileGravityScale = 1.f;
			}
			break;
		}
		case EProjectileType::Falling:
		{
			ProjMovementComp->ProjectileGravityScale = 1.f;
			ProjMovementComp->Velocity = LaunchDir.GetSafeNormal() * ProjectileSpeed;
			break;
		}
		case EProjectileType::Homing:
		{
			if (InitData.HomingTarget.IsValid())
			{
				ProjMovementComp->Velocity = LaunchDir.GetSafeNormal() * ProjectileSpeed*0.5f;
				ProjMovementComp->bIsHomingProjectile = true;
				ProjMovementComp->HomingTargetComponent = InitData.HomingTarget.Get();
				ProjMovementComp->HomingAccelerationMagnitude = ProjectileSpeed * 6.f;
			}
			else
			{
				ProjectileType = EProjectileType::Straight;
				ProjMovementComp->Velocity = InitData.LaunchDir.GetSafeNormal() * ProjectileSpeed;
				StraightTypeInit();
			}
			break;
		}
	}
}

void ACAP_ProjectileBase::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor==GetOwner() || OtherActor == this || OtherActor->GetClass() == this->GetClass())
		return;

	if (ProjectileType == EProjectileType::Straight || ProjectileType==EProjectileType::Homing)
		ProcessStraightHit(OtherActor,SweepResult);
	else
		ProcessExplosiveHit(SweepResult);
	
	//UE_LOG(LogTemp,Warning,TEXT("발생시키는 트리거 태그 = %s"),*HitTriggerTag.ToString());
}

void ACAP_ProjectileBase::TravelMaxDistanceReached()
{
	Destroy();
}

void ACAP_ProjectileBase::SendLocalGameplayCue(const FHitResult& HitResult)
{
	if (HitGameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;
		
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), HitGameplayCueTag, EGameplayCueEvent::Executed, CueParams);
	}
}

void ACAP_ProjectileBase::ProcessStraightHit(AActor* OtherActor, const FHitResult& SweepResult)
{
	if (HitActors.Contains(OtherActor)) return;
	HitActors.Add(OtherActor);

	// 데미지 적용
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC && HitEffectHandle.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*HitEffectHandle.Data.Get());
	}

	// 이벤트 전송
	FGameplayEventData Payload;
	Payload.Instigator = GetInstigator();
	Payload.Target = OtherActor;

	FHitResult FinalHit(OtherActor, nullptr, GetActorLocation(), GetActorForwardVector());
	FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(FinalHit);
	Payload.TargetData.Add(TargetData);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetInstigator(), HitTriggerTag, Payload);

	SendLocalGameplayCue(SweepResult);

	// 관통 횟수 초과 시 파괴
	CurrentHitCount++;
	if (CurrentHitCount >= MaxHitCount)
	{
		GetWorldTimerManager().ClearTimer(ProjTimerHandle);
		Destroy();
	}
}

void ACAP_ProjectileBase::ProcessExplosiveHit(const FHitResult& SweepResult)
{
	if (ExplosionRadius <= 0.f)
	{
		Destroy();
		return;
	}
	
	FGameplayEventData Payload;
	Payload.Instigator = GetInstigator();

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjQueryParams;
	
	uint8 TeamIdVal = 255;
	if (IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(GetInstigator()))
		TeamIdVal = TeamAgent->GetGenericTeamId().GetId();
	
	ECollisionChannel TargetChannel = (TeamIdVal == 0) ? ECC_EnemyHitbox : ECC_PlayerHitbox;
	ObjQueryParams.AddObjectTypesToQuery(TargetChannel);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
	TArray<TWeakObjectPtr<AActor>> HitActorsArray;

	// 폭발 판정 (구체)
	bool bHit = GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, ObjQueryParams, SphereShape);
	if (bHit)
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* OverlapActor = Overlap.GetActor();
			if (OverlapActor && OverlapActor != GetOwner() && !HitActorsArray.Contains(OverlapActor))
			{
				HitActorsArray.Add(OverlapActor);
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlapActor);
				if (TargetASC && HitEffectHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*HitEffectHandle.Data.Get());
				}
			}
		}

		if (HitActorsArray.Num() > 0 && HitActorsArray[0].IsValid())
		{
			Payload.Target = HitActorsArray[0].Get();
			FGameplayAbilityTargetData_ActorArray* TargetDataArray = new FGameplayAbilityTargetData_ActorArray();
			TargetDataArray->TargetActorArray = HitActorsArray;
			Payload.TargetData.Add(TargetDataArray);
		}
	}

	if (Payload.TargetData.Num() > 0)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetInstigator(), HitTriggerTag, Payload);
	}

	SendLocalGameplayCue(SweepResult);
	Destroy();
}

void ACAP_ProjectileBase::StraightTypeInit()
{
	ProjMovementComp->ProjectileGravityScale = 0.f;

	float TravelMaxTime = MaxDistance / ProjectileSpeed;
	GetWorld()->GetTimerManager().SetTimer(ProjTimerHandle, this, &ACAP_ProjectileBase::TravelMaxDistanceReached, TravelMaxTime);
}
