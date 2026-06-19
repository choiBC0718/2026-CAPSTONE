// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/BTTask_RetreatFromTarget.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_RetreatFromTarget::UBTTask_RetreatFromTarget()
{
	NodeName = TEXT("Retreat From Target");
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_RetreatFromTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	if (IsRetreatOnCooldown(Pawn))
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

	const FVector CandidateLocation = BuildRetreatLocation(*Pawn, *TargetActor);
	FVector RetreatLocation = CandidateLocation;
	if (!ProjectToNavigation(*Pawn, CandidateLocation, RetreatLocation))
	{
		return EBTNodeResult::Failed;
	}

	CurrentRetreatLocation = RetreatLocation;
	CurrentTargetActor = TargetActor;
	RetreatElapsedTime = 0.f;

	const EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		RetreatLocation,
		AcceptableRadius,
		true);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_RetreatFromTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	RetreatElapsedTime += DeltaSeconds;
	if (RetreatElapsedTime >= MaxRetreatDuration)
	{
		if (AIController)
		{
			AIController->StopMovement();
		}
		MarkRetreatCooldownStarted(Pawn);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	if (CurrentTargetActor.IsValid())
	{
		const float TargetDistanceSq = FVector::DistSquared2D(Pawn->GetActorLocation(), CurrentTargetActor->GetActorLocation());
		if (TargetDistanceSq >= FMath::Square(DesiredDistanceFromTarget))
		{
			if (AIController)
			{
				AIController->StopMovement();
			}
			MarkRetreatCooldownStarted(Pawn);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}

	const float DistanceSq = FVector::DistSquared2D(Pawn->GetActorLocation(), CurrentRetreatLocation);
	if (DistanceSq <= FMath::Square(AcceptableRadius))
	{
		MarkRetreatCooldownStarted(Pawn);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

FVector UBTTask_RetreatFromTarget::BuildRetreatLocation(const APawn& Pawn, const AActor& TargetActor) const
{
	FVector RetreatDirection = Pawn.GetActorLocation() - TargetActor.GetActorLocation();
	RetreatDirection.Z = 0.f;

	if (RetreatDirection.IsNearlyZero())
	{
		RetreatDirection = -Pawn.GetActorForwardVector();
		RetreatDirection.Z = 0.f;
	}

	RetreatDirection.Normalize();
	return Pawn.GetActorLocation() + RetreatDirection * RetreatDistance;
}

bool UBTTask_RetreatFromTarget::ProjectToNavigation(
	const UObject& WorldContextObject,
	const FVector& CandidateLocation,
	FVector& OutProjectedLocation) const
{
	UWorld* World = WorldContextObject.GetWorld();
	UNavigationSystemV1* NavSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (!NavSystem)
	{
		return false;
	}

	FNavLocation NavLocation;
	const FVector QueryExtent(ProjectionExtentXY, ProjectionExtentXY, ProjectionExtentZ);
	if (!NavSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, QueryExtent))
	{
		return false;
	}

	OutProjectedLocation = NavLocation.Location;
	return true;
}

bool UBTTask_RetreatFromTarget::IsRetreatOnCooldown(const APawn* Pawn) const
{
	if (!Pawn || RetreatCooldown <= 0.f)
	{
		return false;
	}

	const UWorld* World = Pawn->GetWorld();
	const float* LastRetreatEndTime = LastRetreatEndTimeByPawn.Find(Pawn);
	if (!World || !LastRetreatEndTime)
	{
		return false;
	}

	return World->GetTimeSeconds() - *LastRetreatEndTime < RetreatCooldown;
}

void UBTTask_RetreatFromTarget::MarkRetreatCooldownStarted(const APawn* Pawn)
{
	if (!Pawn || RetreatCooldown <= 0.f)
	{
		return;
	}

	if (const UWorld* World = Pawn->GetWorld())
	{
		LastRetreatEndTimeByPawn.FindOrAdd(Pawn) = World->GetTimeSeconds();
	}
}
