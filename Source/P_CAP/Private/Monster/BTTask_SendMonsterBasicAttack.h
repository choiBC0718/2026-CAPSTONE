// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SendMonsterBasicAttack.generated.h"

class UAbilitySystemComponent;

UCLASS()
class UBTTask_SendMonsterBasicAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SendMonsterBasicAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	bool IsBasicAttackActive(const UAbilitySystemComponent* ASC) const;
	bool TryActivateBasicAttack(UAbilitySystemComponent* ASC) const;
	bool IsAttackOnCooldown(const APawn* Pawn) const;
	void MarkAttackCooldownStarted(const APawn* Pawn);

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FName TargetBlackboardKeyName = TEXT("Target");

	UPROPERTY(EditAnywhere, Category="Attack", meta=(ClampMin="0.0"))
	float AttackRange = 180.f;

	UPROPERTY(EditAnywhere, Category="Attack", meta=(ClampMin="0.0"))
	float AttackCooldown = 1.5f;

	UPROPERTY()
	mutable TMap<TObjectPtr<const APawn>, float> LastAttackEndTimeByPawn;
};
