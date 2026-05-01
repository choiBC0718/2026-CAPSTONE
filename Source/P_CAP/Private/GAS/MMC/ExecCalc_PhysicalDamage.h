// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_PhysicalDamage.generated.h"

/**
 * 
 */
UCLASS()
class UExecCalc_PhysicalDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_PhysicalDamage();
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	FGameplayEffectAttributeCaptureDefinition PhysicalDamageCapture;
	FGameplayEffectAttributeCaptureDefinition PhysicalPenetrationCapture;
	FGameplayEffectAttributeCaptureDefinition PhysicalArmorCapture;
	FGameplayEffectAttributeCaptureDefinition CriticalChanceCapture;
	FGameplayEffectAttributeCaptureDefinition CriticalDamageCapture;

	FGameplayTag DamageMultiplierDataTag;
	FGameplayTag ChargeMultiplierDataTag;
};
