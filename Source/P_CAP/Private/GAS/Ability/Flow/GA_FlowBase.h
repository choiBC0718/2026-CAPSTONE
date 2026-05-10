// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GA_FlowBase.generated.h"

/**
 * 스킬 입력 로직 처리 클래스
 * 일회성, Combo, Hold, Charge, Targeting 등
 */
UCLASS()
class UGA_FlowBase : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGA_FlowBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	float RotateToCursorSpeed = 1500.f;

	UFUNCTION()
	void OnRMSTagReceived(FGameplayEventData Payload);
	
	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* MontageTask;
	
	float ChargedTime = 1.f;
};
