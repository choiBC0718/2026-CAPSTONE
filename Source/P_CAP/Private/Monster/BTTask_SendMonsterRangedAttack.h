// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SendMonsterRangedAttack.generated.h"

class UAbilitySystemComponent;

UCLASS()
class UBTTask_SendMonsterRangedAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SendMonsterRangedAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	bool IsRangedAttackActive(const UAbilitySystemComponent* ASC) const;
	bool TryActivateRangedAttack(UAbilitySystemComponent* ASC) const;
	void StopPawnMovement(APawn* Pawn) const;
	bool IsAttackOnCooldown(const APawn* Pawn) const;
	void MarkAttackCooldownStarted(const APawn* Pawn);

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetBlackboardKeyName = TEXT("Target");

	UPROPERTY(EditAnywhere, Category="Attack", meta=(ClampMin="0.0"))
	float AttackRange = 900.f;

	UPROPERTY(EditAnywhere, Category="Attack", meta=(ClampMin="0.0"))
	float AttackCooldown = 2.f;

	UPROPERTY()
	mutable TMap<TObjectPtr<const APawn>, float> LastAttackEndTimeByPawn;
};
