// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/BTTask_SendMonsterRangedAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"

UBTTask_SendMonsterRangedAttack::UBTTask_SendMonsterRangedAttack()
{
	NodeName = TEXT("Send Monster Ranged Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_SendMonsterRangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	if (IsRangedAttackActive(ASC))
	{
		StopPawnMovement(Pawn);
		return EBTNodeResult::InProgress;
	}

	if (IsAttackOnCooldown(Pawn))
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = BlackboardComponent
		? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetBlackboardKeyName))
		: nullptr;
	if (!IsValid(TargetActor))
	{
		return EBTNodeResult::Failed;
	}

	const float DistanceSq = FVector::DistSquared2D(Pawn->GetActorLocation(), TargetActor->GetActorLocation());
	if (DistanceSq > FMath::Square(AttackRange))
	{
		return EBTNodeResult::Failed;
	}

	if (!TryActivateRangedAttack(ASC))
	{
		return EBTNodeResult::Failed;
	}

	AIController->StopMovement();
	StopPawnMovement(Pawn);
	return EBTNodeResult::InProgress;
}

void UBTTask_SendMonsterRangedAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	UAbilitySystemComponent* ASC = Pawn ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn) : nullptr;
	if (ASC && IsRangedAttackActive(ASC))
	{
		if (AIController)
		{
			AIController->StopMovement();
		}
		StopPawnMovement(Pawn);
		return;
	}

	MarkAttackCooldownStarted(Pawn);
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

bool UBTTask_SendMonsterRangedAttack::IsRangedAttackActive(const UAbilitySystemComponent* ASC) const
{
	if (!ASC)
	{
		return false;
	}

	const int32 InputID = static_cast<int32>(EAbilityInputID::BasicAttack);
	for (const FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
	{
		if (AbilitySpec.InputID == InputID && AbilitySpec.IsActive())
		{
			return true;
		}
	}

	return false;
}

bool UBTTask_SendMonsterRangedAttack::TryActivateRangedAttack(UAbilitySystemComponent* ASC) const
{
	if (!ASC)
	{
		return false;
	}

	const int32 InputID = static_cast<int32>(EAbilityInputID::BasicAttack);
	bool bActivatedAbility = false;
	for (FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
	{
		if (AbilitySpec.InputID != InputID)
		{
			continue;
		}

		ASC->AbilitySpecInputPressed(AbilitySpec);
		bActivatedAbility |= ASC->TryActivateAbility(AbilitySpec.Handle);
		ASC->AbilitySpecInputReleased(AbilitySpec);
	}

	return bActivatedAbility || IsRangedAttackActive(ASC);
}

void UBTTask_SendMonsterRangedAttack::StopPawnMovement(APawn* Pawn) const
{
	if (!Pawn)
	{
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
		}
	}
}

bool UBTTask_SendMonsterRangedAttack::IsAttackOnCooldown(const APawn* Pawn) const
{
	if (!Pawn || AttackCooldown <= 0.f)
	{
		return false;
	}

	const UWorld* World = Pawn->GetWorld();
	const float* LastAttackEndTime = LastAttackEndTimeByPawn.Find(Pawn);
	if (!World || !LastAttackEndTime)
	{
		return false;
	}

	return World->GetTimeSeconds() - *LastAttackEndTime < AttackCooldown;
}

void UBTTask_SendMonsterRangedAttack::MarkAttackCooldownStarted(const APawn* Pawn)
{
	if (!Pawn || AttackCooldown <= 0.f)
	{
		return;
	}

	if (const UWorld* World = Pawn->GetWorld())
	{
		LastAttackEndTimeByPawn.FindOrAdd(Pawn) = World->GetTimeSeconds();
	}
}
