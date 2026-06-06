// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/Payload/GA_PayloadBase.h"
#include "GA_Payload_SpawnGroundAoE.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Payload_SpawnGroundAoE : public UGA_PayloadBase
{
	GENERATED_BODY()

protected:
	virtual void ExecutePayloadLogic(const FGameplayEventData& EventData) override;
	virtual float GetPayloadTargetingRadius() override {return AoERadius;}

	UPROPERTY(EditDefaultsOnly, Category="AoE")
	TSubclassOf<class ACAP_OverlapDamageActorBase> AoEActorClass;

	// 장판의 타격 반경 (= 타겟팅 데칼 반경)
	UPROPERTY(EditDefaultsOnly, Category="AoE")
	float AoERadius = 300.f;

	// 장판 지속 시간
	UPROPERTY(EditDefaultsOnly, Category="AoE")
	float LifeSpan = 5.f;

	// 데미지 틱 주기 (0이면 처음 닿을 때 1번만 데미지)
	UPROPERTY(EditDefaultsOnly, Category="AoE")
	float DamageTickRate = 0.5f;
};
