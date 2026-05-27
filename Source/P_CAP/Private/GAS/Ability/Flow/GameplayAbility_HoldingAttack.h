// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_FlowBase.h"
#include "GameplayAbility_HoldingAttack.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_HoldingAttack : public UGA_FlowBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_HoldingAttack();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override; 

protected:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	UFUNCTION()
	void OnMaxHeld();

	void ExecuteAttack();
	
private:
	bool bIsExecuted=false;

	FGameplayTag JogLoopTag;
	UFUNCTION()
	void OnJogFwdTagReceived(FGameplayEventData Payload);

	UPROPERTY()
	class UAbilityTask_WaitInputRelease* InputReleaseTask;
	UPROPERTY()
	class UAbilityTask_WaitDelay* MaxHoldTask;
	UPROPERTY()
	class UAbilityTask_TickMoveToCursor* MoveToCursor;

	void ClearTasks();
};
