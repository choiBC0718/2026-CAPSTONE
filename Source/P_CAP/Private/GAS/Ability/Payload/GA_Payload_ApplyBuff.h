// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/Payload/GA_PayloadBase.h"
#include "Interface/CAP_BuffVisualInterface.h"
#include "GA_Payload_ApplyBuff.generated.h"

USTRUCT(BlueprintType)
struct FSkillStatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(Categories="Data.ItemStat"))
	FGameplayTag StatTag;
	UPROPERTY(EditAnywhere)
	float Value = 0.f;
	UPROPERTY(EditAnywhere)
	float Duration=10.f;
	// false = ADD(합연산) / true = Mul(곱연산)
	UPROPERTY(EditAnywhere)
	bool IsMultiplier=false;
};


/**
 * 
 */
UCLASS()
class UGA_Payload_ApplyBuff : public UGA_PayloadBase, public ICAP_BuffVisualInterface
{
	GENERATED_BODY()

public:
	virtual FBuffDisplayData GetBuffDisplayData(const FGameplayTag& EffectTag) const override;
	
protected:
	virtual void ExecutePayloadLogic(const FGameplayEventData& EventData) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Effect", meta=(TitleProperty="StatTag"))
	TArray<FSkillStatModifier> SkillStatModifiers;
};
