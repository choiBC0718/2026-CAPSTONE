// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_RotateToCursor.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRotateComplete);
/**
 * 
 */
UCLASS()
class UAbilityTask_RotateToCursor : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_RotateToCursor();
	virtual void TickTask(float DeltaTime) override;
	virtual void Activate() override;

	UFUNCTION()
	static UAbilityTask_RotateToCursor* SmoothRotateToMouse(UGameplayAbility* OwningAbility, float RotationSpeed);
	
	UPROPERTY()
	FOnRotateComplete OnComplete;
	
private:
	float InterpSpeed;
	FRotator TargetRotation;
	bool bIsFinished;
};
