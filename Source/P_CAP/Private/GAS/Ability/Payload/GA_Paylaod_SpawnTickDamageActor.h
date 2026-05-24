// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/Payload/GA_PayloadBase.h"
#include "GA_Paylaod_SpawnTickDamageActor.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Paylaod_SpawnTickDamageActor : public UGA_PayloadBase
{
	GENERATED_BODY()

protected:
	virtual void ExecutePayloadLogic(const FGameplayEventData& EventData) override;

	UPROPERTY(EditDefaultsOnly, Category="Spawn")
	TSubclassOf<class ACAP_OverlapDamageActorBase> OverlapDamageActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn")
	float LifeSpan = 5.f;
	UPROPERTY(EditDefaultsOnly, Category="Spawn")
	float CollisionRadius = 80.f;
	UPROPERTY(EditDefaultsOnly, Category="Spawn")
	float DamageTickRate = 0.5f;
};
