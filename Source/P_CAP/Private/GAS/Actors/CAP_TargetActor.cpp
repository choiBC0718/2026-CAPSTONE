// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Actors/CAP_TargetActor.h"

#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"

ACAP_TargetActor::ACAP_TargetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(SceneRoot);
	
	DecalComp = CreateDefaultSubobject<UDecalComponent>("Decal Comp");
	DecalComp->SetupAttachment(GetRootComponent());
}

void ACAP_TargetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!PrimaryPC || !OwningAbility || !OwningAbility->GetAvatarActorFromActorInfo())
		return;
	
	const FVector TargetPoint = GetTargetPoint();
	const FVector CharacterLoc = OwningAbility->GetAvatarActorFromActorInfo()->GetActorLocation();
	const float CurrentDistance = FVector::Dist2D(TargetPoint, CharacterLoc);

	FVector FinalLoc;

	if (CurrentDistance > MaxTargetingRange)
	{
		FVector Direction = TargetPoint - CharacterLoc;
		Direction.Z=0.f;
		Direction.Normalize();

		FVector ClampedLoc = CharacterLoc + (Direction * MaxTargetingRange);
		ClampedLoc.Z = TargetPoint.Z;
		FinalLoc = ClampedLoc;
	}
	else
	{
		FinalLoc = TargetPoint;
	}
	
	SetActorLocation(FinalLoc);
}

void ACAP_TargetActor::ConfirmTargetingAndContinue()
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit();

	FVector TargetLoc = GetActorLocation();
	NewData->HitResult.bBlockingHit = true;
	NewData->HitResult.ImpactPoint = TargetLoc;
	NewData->HitResult.TraceStart = TargetLoc;
	NewData->HitResult.Location = TargetLoc;
	TargetDataHandle.Add(NewData);

	TargetDataReadyDelegate.Broadcast(TargetDataHandle);
}

void ACAP_TargetActor::Initialize(float NewMaxRange, float NewMaxRadius)
{
	MaxTargetingRange = NewMaxRange;
	TargetAreaRadius = NewMaxRadius;
	
	DecalComp->DecalSize = FVector(TargetAreaRadius);
}

FVector ACAP_TargetActor::GetTargetPoint() const
{
	FHitResult HitResult;
	if (PrimaryPC->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		return HitResult.ImpactPoint;
	}
	return GetActorLocation();
}
