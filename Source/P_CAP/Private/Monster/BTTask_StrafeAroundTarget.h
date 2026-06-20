// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StrafeAroundTarget.generated.h"

UCLASS()
class UBTTask_StrafeAroundTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_StrafeAroundTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetBlackboardKeyName = TEXT("Target");

	UPROPERTY(EditAnywhere, Category="Strafe", meta=(ClampMin="0.0"))
	float StrafeDistance = 300.f;

	UPROPERTY(EditAnywhere, Category="Strafe", meta=(ClampMin="0.0"))
	float AcceptableRadius = 80.f;

	UPROPERTY(EditAnywhere, Category="Strafe", meta=(ClampMin="0.1"))
	float MaxStrafeDuration = 0.8f;

	UPROPERTY(EditAnywhere, Category="Strafe", meta=(ClampMin="0.0"))
	float ProjectionExtentXY = 500.f;

	UPROPERTY(EditAnywhere, Category="Strafe", meta=(ClampMin="0.0"))
	float ProjectionExtentZ = 200.f;

	FVector BuildStrafeLocation(const APawn& Pawn, const AActor& TargetActor) const;
	bool ProjectToNavigation(const UObject& WorldContextObject, const FVector& CandidateLocation, FVector& OutProjectedLocation) const;

	FVector CurrentStrafeLocation = FVector::ZeroVector;
	float StrafeElapsedTime = 0.f;
};
