// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CAP_AIController.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_AIController : public AAIController
{
	GENERATED_BODY()

public:
	ACAP_AIController();
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category="AI Behavior")
	FName TargetBlackboardKeyName = "Target";
	UPROPERTY(EditDefaultsOnly, Category="AI Behavior")
	class UBehaviorTree* BehaviorTree;
	
	UPROPERTY(EditDefaultsOnly, Category="Perception")
	class UAIPerceptionComponent* AIPerceptionComp;
	
	UPROPERTY(EditDefaultsOnly, Category="Perception")
	class UAISense_Sight* SightConfig;


	void ClearAndDisableAllSenses();
	void EnableAllSenses();
};
