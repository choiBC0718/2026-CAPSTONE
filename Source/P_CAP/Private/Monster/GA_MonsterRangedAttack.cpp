// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/GA_MonsterRangedAttack.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AN_SpawnProjectile.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GameFramework/Character.h"
#include "GameplayEffect.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"

UGA_MonsterRangedAttack::UGA_MonsterRangedAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	AbilityTags.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());
	BlockAbilitiesWithTag.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());
}

void UGA_MonsterRangedAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CurrentAttackEntry = ResolveAttackEntry();
	CurrentAttackMontage = CurrentAttackEntry.Montage;

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (!CurrentAttackMontage || !ProjectileClass)
	{
		K2_EndAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CurrentAttackMontage,
		CurrentAttackEntry.PlayRate);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MonsterRangedAttack::OnAttackMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_MonsterRangedAttack::OnAttackMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MonsterRangedAttack::OnAttackMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MonsterRangedAttack::OnAttackMontageFinished);
	MontageTask->ReadyForActivation();

	if (UWorld* World = GetWorld())
	{
		const float SafePlayRate = FMath::Max(0.1f, CurrentAttackEntry.PlayRate);
		const float FallbackDuration = CurrentAttackMontage->GetPlayLength() / SafePlayRate + 0.25f;
		World->GetTimerManager().SetTimer(
			AttackFallbackTimerHandle,
			this,
			&UGA_MonsterRangedAttack::EndAttackByFallbackTimer,
			FallbackDuration,
			false);
	}

	UAbilityTask_WaitGameplayEvent* WaitProjectileEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		UCAP_AbilitySystemStatics::GetSpawnProjectileTag());
	WaitProjectileEventTask->EventReceived.AddDynamic(this, &UGA_MonsterRangedAttack::OnSpawnProjectileEventReceived);
	WaitProjectileEventTask->ReadyForActivation();
}

void UGA_MonsterRangedAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	CurrentAttackMontage = nullptr;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackFallbackTimerHandle);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_MonsterRangedAttack::OnSpawnProjectileEventReceived(FGameplayEventData Payload)
{
	SpawnProjectileAtTarget(Payload);
}

void UGA_MonsterRangedAttack::OnAttackMontageFinished()
{
	K2_EndAbility();
}

FMonsterRangedAttackEntry UGA_MonsterRangedAttack::ResolveAttackEntry() const
{
	float TotalWeight = 0.f;
	for (const FMonsterRangedAttackEntry& Entry : AttackEntries)
	{
		if (Entry.Montage && Entry.Weight > 0.f)
		{
			TotalWeight += Entry.Weight;
		}
	}

	if (TotalWeight > 0.f)
	{
		const float Roll = FMath::FRandRange(0.f, TotalWeight);
		float AccumulatedWeight = 0.f;
		for (const FMonsterRangedAttackEntry& Entry : AttackEntries)
		{
			if (!Entry.Montage || Entry.Weight <= 0.f)
			{
				continue;
			}

			AccumulatedWeight += Entry.Weight;
			if (Roll <= AccumulatedWeight)
			{
				return Entry;
			}
		}
	}

	FMonsterRangedAttackEntry FallbackEntry;
	FallbackEntry.Montage = AttackMontage;
	FallbackEntry.BaseDamage = BaseDamage;
	FallbackEntry.DamageMultiplier = DamageMultiplier;
	FallbackEntry.DamageType = DamageType;
	FallbackEntry.PlayRate = MontagePlayRate;
	return FallbackEntry;
}

bool UGA_MonsterRangedAttack::SpawnProjectileAtTarget(const FGameplayEventData& Payload)
{
	UWorld* World = GetWorld();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!World || !AvatarActor || !ProjectileClass)
	{
		return false;
	}

	const FVector SpawnLocation = ResolveSpawnLocation(Payload);
	const FRotator SpawnRotation = ResolveFireRotation(SpawnLocation);
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACAP_ProjectileBase* Projectile = World->SpawnActorDeferred<ACAP_ProjectileBase>(
		ProjectileClass,
		SpawnTransform,
		SpawnParams.Owner,
		SpawnParams.Instigator,
		SpawnParams.SpawnCollisionHandlingOverride);
	if (!Projectile)
	{
		return false;
	}

	FProjectileInitData InitData;
	InitData.ProjectileType = ProjectileType;
	InitData.LaunchDir = SpawnRotation.Vector();
	InitData.ProjectileSpeed = ProjectileSpeed;
	InitData.MaxDistance = MaxDistance;
	InitData.ExplosionRadius = ExplosionRadius;
	InitData.MaxHitCount = MaxHitCount;
	InitData.ArcTension = ArcTension;
	InitData.DamageSpecHandle = MakeDamageSpec();
	InitData.TargetActorClass = ACAP_PlayerCharacter::StaticClass();
	if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		InitData.TargetLocation = PlayerPawn->GetActorLocation();
	}
	else
	{
		InitData.TargetLocation = SpawnLocation + SpawnRotation.Vector() * MaxDistance;
	}

	Projectile->InitProjectile(InitData);
	Projectile->FinishSpawning(SpawnTransform);
	return true;
}

FVector UGA_MonsterRangedAttack::ResolveSpawnLocation(const FGameplayEventData& Payload) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return FVector::ZeroVector;
	}

	if (const ACharacter* Character = Cast<ACharacter>(AvatarActor))
	{
		FName SocketToUse = FireSocketName;
		if (const UAN_SpawnProjectile* NotifyData = Cast<UAN_SpawnProjectile>(Payload.OptionalObject))
		{
			SocketToUse = NotifyData->SocketName;
		}

		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (Mesh->DoesSocketExist(SocketToUse))
			{
				return Mesh->GetSocketLocation(SocketToUse);
			}
		}
	}

	return AvatarActor->GetActorLocation() + AvatarActor->GetActorRotation().RotateVector(SpawnOffset);
}

FRotator UGA_MonsterRangedAttack::ResolveFireRotation(const FVector& SpawnLocation) const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return AvatarActor ? AvatarActor->GetActorRotation() : FRotator::ZeroRotator;
	}

	FVector AimDirection = PlayerPawn->GetActorLocation() - SpawnLocation;
	AimDirection.Z = 0.f;
	if (AimDirection.IsNearlyZero())
	{
		return AvatarActor ? AvatarActor->GetActorRotation() : FRotator::ZeroRotator;
	}

	return AimDirection.Rotation();
}

FGameplayEffectSpecHandle UGA_MonsterRangedAttack::MakeDamageSpec() const
{
	UCAP_AbilitySystemComponent* SourceASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!SourceASC || !SourceASC->GetGenerics())
	{
		return FGameplayEffectSpecHandle();
	}

	TSubclassOf<UGameplayEffect> DamageGE = SourceASC->GetGenerics()->GetInstantDamageGE(CurrentAttackEntry.DamageType);
	if (!DamageGE)
	{
		return FGameplayEffectSpecHandle();
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGE, GetAbilityLevel(), Context);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(UCAP_AbilitySystemStatics::GetDataDamageBaseTag(), CurrentAttackEntry.BaseDamage);
		SpecHandle.Data->SetSetByCallerMagnitude(UCAP_AbilitySystemStatics::GetDataDamageMultiplierTag(), CurrentAttackEntry.DamageMultiplier);
	}

	return SpecHandle;
}

void UGA_MonsterRangedAttack::EndAttackByFallbackTimer()
{
	K2_EndAbility();
}
