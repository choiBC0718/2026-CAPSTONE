// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/BTTask_SendMonsterBasicAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"

UBTTask_SendMonsterBasicAttack::UBTTask_SendMonsterBasicAttack()
{
	NodeName = TEXT("Send Monster Basic Attack");
}

EBTNodeResult::Type UBTTask_SendMonsterBasicAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	const int32 InputID = static_cast<int32>(EAbilityInputID::BasicAttack);
	bool bActivatedAbility = false;
	for (FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
	{
		if (AbilitySpec.InputID != InputID)
		{
			continue;
		}

		if (AbilitySpec.IsActive())
		{
			return EBTNodeResult::Succeeded;
		}

		ASC->AbilitySpecInputPressed(AbilitySpec);
		bActivatedAbility |= ASC->TryActivateAbility(AbilitySpec.Handle);
		ASC->AbilitySpecInputReleased(AbilitySpec);
	}

	if (!bActivatedAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("Monster basic attack failed: no BasicAttack ability activated. Pawn=%s"), *GetNameSafe(Pawn));
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;
}
