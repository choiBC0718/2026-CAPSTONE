// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_TickMoveToCursor.generated.h"

/**
 * 
 */
UCLASS()
class UAbilityTask_TickMoveToCursor : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_TickMoveToCursor();
	virtual void TickTask(float DeltaTime) override;
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

	UFUNCTION()
	static UAbilityTask_TickMoveToCursor* MoveToCursor(UGameplayAbility* OwningAbility, float InSpeedMultiplier = 1.0f);

private:
	float SpeedMultiplier;
	bool bIsFinished;
	float OriginalMaxWalkSpeed;
};
