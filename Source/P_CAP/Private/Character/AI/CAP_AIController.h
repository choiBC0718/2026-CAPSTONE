// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "CAP_AIController.generated.h"

struct FAIStimulus;

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

	UFUNCTION(BlueprintCallable, Category="AI Behavior")
	void SetAIEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="AI Behavior")
	void SetTargetActor(AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category="AI Behavior")
	bool IsAIEnabled() const { return bAIEnabled; }

private:
	UPROPERTY(EditDefaultsOnly, Category="AI Behavior")
	FName TargetBlackboardKeyName = "Target";

	UPROPERTY(EditDefaultsOnly, Category="AI Behavior")
	FName AIEnabledBlackboardKeyName = "bAIEnabled";

	UPROPERTY(EditDefaultsOnly, Category="AI Behavior")
	class UBehaviorTree* BehaviorTree;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Perception")
	class UAIPerceptionComponent* AIPerceptionComp;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Perception")
	class UAISenseConfig_Sight* SightConfig;

	UPROPERTY(VisibleInstanceOnly, Category="AI Behavior")
	bool bAIEnabled = false;

	UPROPERTY()
	TObjectPtr<AActor> CachedTargetActor;

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);

	UFUNCTION()
	void TargetForgotten(AActor* ForgottenActor);

	const UObject* GetCurrentTarget() const;
	void SetCurrentTarget(AActor* NewTarget);
	AActor* GetNextPerceivedActor() const;
	void ForgetActorIfDead(AActor* ActorToForget);

	void ClearAndDisableAllSenses();
	void EnableAllSenses();
	void ApplyBlackboardValues();
};
