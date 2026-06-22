// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Actors/CAP_OverlapDamageActorBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Components/SphereComponent.h"
#include "P_CAP/P_CAP.h"

ACAP_OverlapDamageActorBase::ACAP_OverlapDamageActorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>("CollisionComponent");
	SetRootComponent(CollisionComp);
	CollisionComp->SetCollisionProfileName(FName("Projectile_AoE"));
	CollisionComp->SetCollisionResponseToChannel(ECC_EnemyHitbox, ECR_Overlap);
}

void ACAP_OverlapDamageActorBase::InitSkillActor(const FOverlapDamageActorInitData& InitData)
{
	SkillData = InitData;
	SetLifeSpan(SkillData.LifeSpan);
	CollisionComp->SetSphereRadius(SkillData.CollisionRadius);

	if (SkillData.DamageTickRate > 0.f)
		GetWorldTimerManager().SetTimer(DamageTickTimerHandle, this, &ACAP_OverlapDamageActorBase::ProcessDamageTick, SkillData.DamageTickRate, true,0.f);
}


void ACAP_OverlapDamageActorBase::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ACAP_OverlapDamageActorBase::OnBeginOverlap);
	CollisionComp->OnComponentEndOverlap.AddDynamic(this, &ACAP_OverlapDamageActorBase::OnEndOverlap);
}

void ACAP_OverlapDamageActorBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor==GetInstigator() || OtherActor==this)
		return;
	
	OverlappingActors.Add(OtherActor);
	if (SkillData.DamageTickRate <= 0.f)
		ProcessDamageTick();
}

void ACAP_OverlapDamageActorBase::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OverlappingActors.Contains(OtherActor))
		OverlappingActors.Remove(OtherActor);
}

void ACAP_OverlapDamageActorBase::ProcessDamageTick()
{
	for (auto It = OverlappingActors.CreateIterator(); It; ++It)
	{
		AActor* Target = *It;

		if (!IsValid(Target))
		{
			It.RemoveCurrent();
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
		if (TargetASC && SkillData.DamageSpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*SkillData.DamageSpecHandle.Data.Get());

			if (SkillData.CueTag.IsValid())
			{
				FGameplayCueParameters CueParams;
				CueParams.Location = Target->GetActorLocation();
				CueParams.Normal = Target->GetActorLocation();
				CueParams.Instigator = GetInstigator();
				
				UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(Target, SkillData.CueTag, EGameplayCueEvent::Executed, CueParams);
			}
		}
	}
}
