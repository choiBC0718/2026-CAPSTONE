// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_ItemDamage.generated.h"

/**
 * 
 */
UCLASS()
class UExecCalc_ItemDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_ItemDamage();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	FGameplayEffectAttributeCaptureDefinition PhysicalArmorCapture;
	FGameplayEffectAttributeCaptureDefinition MagicalArmorCapture;
	FGameplayEffectAttributeCaptureDefinition PhysicalPenCapture;
	FGameplayEffectAttributeCaptureDefinition MagicalPenCapture;
	FGameplayEffectAttributeCaptureDefinition CriticalChanceCapture;
	FGameplayEffectAttributeCaptureDefinition CriticalDamageCapture;
};
