// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_AbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UCAP_AbilitySystemComponent();
	void InitComponent(FName StatRowName);
	void ApplyFullStatEffect();

	FORCEINLINE const class UCAP_AbilitySystemGenerics* GetGenerics() const {return AbilitySystemGenerics;}
	
private:
	void ApplyInitialEffects();
	void InitializeBaseAttribute(FName StatRowName);
	void GiveInitialAbilities();
	
	void ApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level=1);
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbility;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	class UCAP_AbilitySystemGenerics* AbilitySystemGenerics;
	
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
};
