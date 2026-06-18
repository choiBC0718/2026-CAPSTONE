// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/GA_MonsterBasicAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"

UGA_MonsterBasicAttack::UGA_MonsterBasicAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	AbilityTags.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());
	BlockAbilitiesWithTag.AddTag(UCAP_AbilitySystemStatics::GetBasicAttackTag());
}

void UGA_MonsterBasicAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	DamagedActors.Empty();

	if (!K2_CommitAbility())
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_MonsterBasicAttack failed: CommitAbility failed. Avatar=%s"), *GetNameSafe(GetAvatarActorFromActorInfo()));
		K2_EndAbility();
		return;
	}

	if (!AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_MonsterBasicAttack failed: AttackMontage is missing. Avatar=%s"), *GetNameSafe(GetAvatarActorFromActorInfo()));
		K2_EndAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage,
		MontagePlayRate);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MonsterBasicAttack::OnAttackMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_MonsterBasicAttack::OnAttackMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MonsterBasicAttack::OnAttackMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MonsterBasicAttack::OnAttackMontageFinished);
	MontageTask->ReadyForActivation();

	if (UWorld* World = GetWorld())
	{
		const float SafePlayRate = FMath::Max(0.1f, MontagePlayRate);
		const float FallbackDuration = AttackMontage->GetPlayLength() / SafePlayRate + 0.25f;
		World->GetTimerManager().SetTimer(
			AttackFallbackTimerHandle,
			this,
			&UGA_MonsterBasicAttack::EndAttackByFallbackTimer,
			FallbackDuration,
			false);
	}

	UAbilityTask_WaitGameplayEvent* WaitHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		UCAP_AbilitySystemStatics::GetAnimHitTag());
	WaitHitEventTask->EventReceived.AddDynamic(this, &UGA_MonsterBasicAttack::OnAnimHitEventReceived);
	WaitHitEventTask->ReadyForActivation();
}

void UGA_MonsterBasicAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	DamagedActors.Empty();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackFallbackTimerHandle);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_MonsterBasicAttack::OnAnimHitEventReceived(FGameplayEventData Payload)
{
	const int32 HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(Payload.TargetData);
	for (int32 HitIndex = 0; HitIndex < HitResultCount; ++HitIndex)
	{
		const FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(Payload.TargetData, HitIndex);
		ApplyDamageFromHitResult(HitResult);
	}
}

void UGA_MonsterBasicAttack::OnAttackMontageFinished()
{
	K2_EndAbility();
}

void UGA_MonsterBasicAttack::EndAttackByFallbackTimer()
{
	UE_LOG(LogTemp, Verbose, TEXT("GA_MonsterBasicAttack fallback ended ability. Avatar=%s Montage=%s"),
		*GetNameSafe(GetAvatarActorFromActorInfo()),
		*GetNameSafe(AttackMontage));
	K2_EndAbility();
}

bool UGA_MonsterBasicAttack::ApplyDamageFromHitResult(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor) || DamagedActors.Contains(HitActor))
	{
		return false;
	}

	if (!HitActor->IsA<ACAP_PlayerCharacter>())
	{
		return false;
	}

	UCAP_AbilitySystemComponent* SourceASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!SourceASC || !TargetASC || !SourceASC->GetGenerics())
	{
		return false;
	}

	TSubclassOf<UGameplayEffect> DamageGE = SourceASC->GetGenerics()->GetInstantDamageGE(DamageType);
	if (!DamageGE)
	{
		return false;
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
	Context.AddHitResult(HitResult);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGE, GetAbilityLevel(), Context);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(UCAP_AbilitySystemStatics::GetDataDamageBaseTag(), BaseDamage);
	SpecHandle.Data->SetSetByCallerMagnitude(UCAP_AbilitySystemStatics::GetDataDamageMultiplierTag(), DamageMultiplier);
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	DamagedActors.Add(HitActor);
	return true;
}
