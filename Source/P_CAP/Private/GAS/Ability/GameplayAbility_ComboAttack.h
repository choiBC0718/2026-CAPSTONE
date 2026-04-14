// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GameplayAbility_ComboAttack.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_ComboAttack : public UCAP_GameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_ComboAttack();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	void SetupWaitComoInputPress();
	void TryCommitCombo();
	
	UFUNCTION()
	void OnNextComboTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void HandleInputPress(float TimeWaited);
	UFUNCTION()
	void OnRotateTagReceived(FGameplayEventData Payload);
	
	FGameplayTag ComboChangeTag;
	FGameplayTag ComboEndTag;
	FGameplayTag RotateTag;

	FName NextComboSectionName;
};
