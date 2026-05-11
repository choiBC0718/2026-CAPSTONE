// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/Payload/GA_PayloadBase.h"
#include "GA_Payload_InstantDamage.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Payload_InstantDamage : public UGA_PayloadBase
{
	GENERATED_BODY()

public:
	UGA_Payload_InstantDamage();

protected:
	virtual void ExecutePayloadLogic(const FGameplayEventData& EventData) override;
};
