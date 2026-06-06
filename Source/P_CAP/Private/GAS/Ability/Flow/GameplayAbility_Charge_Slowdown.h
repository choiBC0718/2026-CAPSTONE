// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/Flow/GA_FlowBase.h"
#include "GameplayAbility_Charge_Slowdown.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_Charge_Slowdown : public UGA_FlowBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Charge_Slowdown();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	bool bIsExecuted = false;
	FGameplayTag ChargeStartTag;

	UFUNCTION()
	void OnChargeStartTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	UFUNCTION()
	void OnMaxCharged();

	UPROPERTY()
	class UAbilityTask_TickRotToCursor* TickRotTask;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float MontageSpeedRateAtCharging = 0.3f;
	// 회전 속도
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float TickRotSpeed = 100.f;
	// 최대 차징 시간
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float MaxChargeTime = 3.f;

	void ExecuteAttack(float ChargeTime=1.f);
};
