// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/BTDecorator_CheckTargetDistance.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CheckTargetDistance::UBTDecorator_CheckTargetDistance()
{
	NodeName = TEXT("Check Target Distance");
}

bool UBTDecorator_CheckTargetDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	const AActor* TargetActor = BlackboardComponent
		? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetBlackboardKeyName))
		: nullptr;

	if (!Pawn || !IsValid(TargetActor))
	{
		return false;
	}

	const float Distance2D = FVector::Dist2D(Pawn->GetActorLocation(), TargetActor->GetActorLocation());
	switch (CheckMode)
	{
	case ETargetDistanceCheckMode::LessOrEqual:
		return Distance2D <= Distance;

	case ETargetDistanceCheckMode::GreaterOrEqual:
		return Distance2D >= Distance;

	case ETargetDistanceCheckMode::Between:
		return Distance2D >= MinDistance && Distance2D <= MaxDistance;

	default:
		return false;
	}
}
