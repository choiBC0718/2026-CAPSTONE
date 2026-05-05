// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_PhysicalDamage.generated.h"

/**
 * 
 */
UCLASS()
class UMMC_PhysicalDamage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_PhysicalDamage();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition PhysicalDamageCapture;
	FGameplayEffectAttributeCaptureDefinition PhysicalPenetrationCapture;
	FGameplayEffectAttributeCaptureDefinition PhysicalArmorCapture;
};
