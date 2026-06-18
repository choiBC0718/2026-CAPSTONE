// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SendMonsterBasicAttack.generated.h"

UCLASS()
class UBTTask_SendMonsterBasicAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SendMonsterBasicAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetBlackboardKeyName = TEXT("Target");

	UPROPERTY(EditAnywhere, Category="Attack", meta=(ClampMin="0.0"))
	float AttackRange = 180.f;
};
