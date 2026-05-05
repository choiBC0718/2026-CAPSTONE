// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "CAP_TargetActor.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_TargetActor : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	ACAP_TargetActor();

	virtual void Tick(float DeltaTime) override;
	virtual void ConfirmTargetingAndContinue() override;

	void Initialize(float NewMaxRange, float NewMaxRadius);
	
protected:
	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* DecalComp;

private:
	float MaxTargetingRange = 1000.f;
	float TargetAreaRadius = 300.f;
	
	FVector GetTargetPoint() const;
};
