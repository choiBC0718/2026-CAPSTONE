// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/BTTask_StrafeAroundTarget.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_StrafeAroundTarget::UBTTask_StrafeAroundTarget()
{
	NodeName = TEXT("Strafe Around Target");
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_StrafeAroundTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	const AActor* TargetActor = BlackboardComponent
		? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetBlackboardKeyName))
		: nullptr;
	if (!IsValid(TargetActor))
	{
		return EBTNodeResult::Failed;
	}

	const FVector CandidateLocation = BuildStrafeLocation(*Pawn, *TargetActor);
	FVector StrafeLocation = CandidateLocation;
	if (!ProjectToNavigation(*Pawn, CandidateLocation, StrafeLocation))
	{
		return EBTNodeResult::Failed;
	}

	CurrentStrafeLocation = StrafeLocation;
	StrafeElapsedTime = 0.f;

	const EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		StrafeLocation,
		AcceptableRadius,
		true);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	return MoveResult == EPathFollowingRequestResult::AlreadyAtGoal
		? EBTNodeResult::Succeeded
		: EBTNodeResult::InProgress;
}

void UBTTask_StrafeAroundTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	StrafeElapsedTime += DeltaSeconds;
	if (StrafeElapsedTime >= MaxStrafeDuration)
	{
		if (AIController)
		{
			AIController->StopMovement();
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	const float DistanceSq = FVector::DistSquared2D(Pawn->GetActorLocation(), CurrentStrafeLocation);
	if (DistanceSq <= FMath::Square(AcceptableRadius))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

FVector UBTTask_StrafeAroundTarget::BuildStrafeLocation(const APawn& Pawn, const AActor& TargetActor) const
{
	FVector ToTarget = TargetActor.GetActorLocation() - Pawn.GetActorLocation();
	ToTarget.Z = 0.f;

	if (ToTarget.IsNearlyZero())
	{
		ToTarget = Pawn.GetActorForwardVector();
		ToTarget.Z = 0.f;
	}

	ToTarget.Normalize();
	FVector StrafeDirection = FVector::CrossProduct(FVector::UpVector, ToTarget).GetSafeNormal();
	if (FMath::RandBool())
	{
		StrafeDirection *= -1.f;
	}

	return Pawn.GetActorLocation() + StrafeDirection * StrafeDistance;
}

bool UBTTask_StrafeAroundTarget::ProjectToNavigation(
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
