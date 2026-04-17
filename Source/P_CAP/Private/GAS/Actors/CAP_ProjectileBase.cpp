// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Actors/CAP_ProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


ACAP_ProjectileBase::ACAP_ProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>("Collision Comp");
	SetRootComponent(CollisionComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("Mesh Component");
	MeshComp->SetupAttachment(CollisionComp);
	
	ProjMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement Component");
	ProjMovementComp->UpdatedComponent = CollisionComp;
	ProjMovementComp->InitialSpeed = 1000.f;
    ProjMovementComp->MaxSpeed = 1000.f;
	ProjMovementComp->bRotationFollowsVelocity = true;
}

void ACAP_ProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ACAP_ProjectileBase::OnOverlapBegin);
}


void ACAP_ProjectileBase::InitStraightProjectile(FVector Direction, FGameplayEffectSpecHandle InHitEffectSpecHandle, FGameplayTag CueTag)
{
	HitGameplayCueTag = CueTag;
	HitEffectHandle = InHitEffectSpecHandle;

	ProjMovementComp->ProjectileGravityScale = 0.f;
	ProjMovementComp->Velocity = Direction.GetSafeNormal() * ProjectileSpeed;

	float TravelMaxTime = MaxDistance / ProjectileSpeed;
	GetWorld()->GetTimerManager().SetTimer(ProjTimerHandle, this, &ACAP_ProjectileBase::TravelMaxDistanceReached, TravelMaxTime);
}

void ACAP_ProjectileBase::InitArcProjectile(FVector TargetLoc, float ArcTension,float InExplosionRadius,
	FGameplayEffectSpecHandle InHitEffectSpecHandle, FGameplayTag CueTag)
{
	HitGameplayCueTag = CueTag;
	HitEffectHandle = InHitEffectSpecHandle;
	ExplosionRadius = InExplosionRadius;

	ProjMovementComp->ProjectileGravityScale = 1.f;
	FVector LaunchVel;
	bool bSuccess = UGameplayStatics::SuggestProjectileVelocity_CustomArc(this, LaunchVel, GetActorLocation(), TargetLoc, 0.f, ArcTension);
	if (bSuccess)
	{
		ProjMovementComp->Velocity = LaunchVel;
	}
	else
	{
		InitStraightProjectile((TargetLoc-GetActorLocation()), InHitEffectSpecHandle, CueTag);
	}
}

void ACAP_ProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor==GetOwner() || OtherActor == this)
		return;

	FHitResult Hit;
	Hit.ImpactPoint = GetActorLocation();
	Hit.ImpactNormal = GetActorForwardVector();
	Hit.Location = GetActorLocation();
	if (ExplosionRadius > 0.f)
	{
		TArray<FOverlapResult> Overlaps;
		FCollisionObjectQueryParams ObjQueryParams;
		ObjQueryParams.AddObjectTypesToQuery(ECC_Pawn);

		FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
		
		bool bHit = GetWorld()->OverlapMultiByObjectType(Overlaps, Hit.ImpactPoint,FQuat::Identity, ObjQueryParams, SphereShape);
		if (bHit)
		{
			for (const FOverlapResult& Overlap : Overlaps)
			{
				AActor* OverlapActor = Overlap.GetActor();
				if (OverlapActor && OverlapActor != GetOwner())
				{
					UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlapActor);
					if (TargetASC && HitEffectHandle.IsValid())
					{
						//FGameplayEffectContextHandle EffectContext = HitEffectHandle.Data->GetContext();
						//EffectContext.AddHitResult(Hit);
						TargetASC->ApplyGameplayEffectSpecToSelf(*HitEffectHandle.Data.Get());
					}
				}
			}
		}
	}
	else
	{
		UAbilitySystemComponent* OtherASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (OtherASC && HitEffectHandle.IsValid())
		{
			//FGameplayEffectContextHandle EffectContext = HitEffectHandle.Data->GetContext();
			//EffectContext.AddHitResult(Hit);
			OtherASC->ApplyGameplayEffectSpecToSelf(*HitEffectHandle.Data.Get());
		}
	}
	SendLocalGameplayCue(OtherActor,Hit);
	GetWorldTimerManager().ClearTimer(ProjTimerHandle);
	Destroy();
}

void ACAP_ProjectileBase::TravelMaxDistanceReached()
{
	Destroy();
}

void ACAP_ProjectileBase::SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult)
{
	if (HitGameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal= HitResult.ImpactNormal;

		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(CueTargetActor, HitGameplayCueTag,EGameplayCueEvent::Executed, CueParams);
	}
}

