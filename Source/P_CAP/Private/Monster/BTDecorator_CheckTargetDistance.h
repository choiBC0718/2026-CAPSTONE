// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckTargetDistance.generated.h"

UENUM(BlueprintType)
enum class ETargetDistanceCheckMode : uint8
{
	LessOrEqual,
	GreaterOrEqual,
	Between,
};

UCLASS()
class UBTDecorator_CheckTargetDistance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckTargetDistance();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetBlackboardKeyName = TEXT("Target");

	UPROPERTY(EditAnywhere, Category="Distance")
	ETargetDistanceCheckMode CheckMode = ETargetDistanceCheckMode::LessOrEqual;

	UPROPERTY(EditAnywhere, Category="Distance", meta=(ClampMin="0.0"))
	float Distance = 450.f;

	UPROPERTY(EditAnywhere, Category="Distance", meta=(ClampMin="0.0", EditCondition="CheckMode==ETargetDistanceCheckMode::Between", EditConditionHides))
	float MinDistance = 450.f;

	UPROPERTY(EditAnywhere, Category="Distance", meta=(ClampMin="0.0", EditCondition="CheckMode==ETargetDistanceCheckMode::Between", EditConditionHides))
	float MaxDistance = 900.f;
};
