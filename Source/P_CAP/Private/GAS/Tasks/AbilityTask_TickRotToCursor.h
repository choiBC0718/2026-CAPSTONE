// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_TickRotToCursor.generated.h"

/**
 * 
 */
UCLASS()
class UAbilityTask_TickRotToCursor : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_TickRotToCursor();
	virtual void TickTask(float DeltaTime) override;

	UFUNCTION()
	static UAbilityTask_TickRotToCursor* TickRotToCursor(UGameplayAbility* OwningAbility, float RotationSpeed);

private:
	float InterpSpeed;
	FRotator TargetRotation;
};
