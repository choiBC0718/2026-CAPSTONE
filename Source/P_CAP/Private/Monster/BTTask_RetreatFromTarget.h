// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RetreatFromTarget.generated.h"

UCLASS()
class UBTTask_RetreatFromTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RetreatFromTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetBlackboardKeyName = TEXT("Target");

	UPROPERTY(EditAnywhere, Category="Retreat", meta=(ClampMin="0.0"))
	float RetreatDistance = 500.f;

	UPROPERTY(EditAnywhere, Category="Retreat", meta=(ClampMin="0.0"))
	float AcceptableRadius = 80.f;

	UPROPERTY(EditAnywhere, Category="Retreat", meta=(ClampMin="0.0"))
	float ProjectionExtentXY = 500.f;

	UPROPERTY(EditAnywhere, Category="Retreat", meta=(ClampMin="0.0"))
	float ProjectionExtentZ = 200.f;

	UPROPERTY(EditAnywhere, Category="Retreat", meta=(ClampMin="0.0"))
	float DesiredDistanceFromTarget = 650.f;

	UPROPERTY(EditAnywhere, Category="Retreat", meta=(ClampMin="0.1"))
	float MaxRetreatDuration = 1.0f;

	UPROPERTY(EditAnywhere, Category="Retreat", meta=(ClampMin="0.0"))
	float RetreatCooldown = 2.5f;

	FVector BuildRetreatLocation(const APawn& Pawn, const AActor& TargetActor) const;
	bool ProjectToNavigation(const UObject& WorldContextObject, const FVector& CandidateLocation, FVector& OutProjectedLocation) const;
	bool IsRetreatOnCooldown(const APawn* Pawn) const;
	void MarkRetreatCooldownStarted(const APawn* Pawn);

	FVector CurrentRetreatLocation = FVector::ZeroVector;
	TWeakObjectPtr<AActor> CurrentTargetActor;
	float RetreatElapsedTime = 0.f;

	UPROPERTY()
	mutable TMap<TObjectPtr<const APawn>, float> LastRetreatEndTimeByPawn;
};
