// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GameplayAbility_ChargeAttack.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_ChargeAttack : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_ChargeAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
private:
	bool bIsExecuted=false;
	FGameplayTag ChargeStartTag;

	UFUNCTION()
	void OnChargeStartTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	UFUNCTION()
	void OnMaxCharged();

	void ExecuteAttack(float ChargeTime=1.f);

	UPROPERTY()
	class UAbilityTask_TickRotToCursor* TickRotTask;

	float ChargeStartTime = 0.f;
	float MaxChargeTime = 3.f;
};
