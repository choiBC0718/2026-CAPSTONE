// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_MagicalDamage.generated.h"

/**
 * 
 */
UCLASS()
class UExecCalc_MagicalDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_MagicalDamage();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	FGameplayEffectAttributeCaptureDefinition MagicalDamageCapture;
	FGameplayEffectAttributeCaptureDefinition MagicalPenetrationCapture;
	FGameplayEffectAttributeCaptureDefinition MagicalArmorCapture;
	FGameplayEffectAttributeCaptureDefinition CriticalChanceCapture;
	FGameplayEffectAttributeCaptureDefinition CriticalDamageCapture;

	FGameplayTag DamageMultiplierDataTag;
	FGameplayTag DamageBaseDataTag;
	FGameplayTag ChargeMultiplierDataTag;
};
