// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_FlowBase.h"
#include "GameplayAbility_ChargeAttack.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_ChargeAttack : public UGA_FlowBase
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

	// 회전 속도
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float TickRotSpeed = 100.f;
	// 최대 차징 시간
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	float MaxChargeTime = 3.f;
};
