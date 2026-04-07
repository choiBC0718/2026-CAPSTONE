// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Character.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_Character.generated.h"

UCLASS()
class ACAP_Character : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACAP_Character();

private:
	/**		Components		**/
	UPROPERTY(VisibleAnywhere, Category="Ability")
	class UCAP_AbilitySystemComponent* CAPAbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, Category="Ability")
	class UCAP_AttributeSet* CAPAttributeSet;

protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(EditDefaultsOnly, Category="Stat")
	FName CharacterStatRowName = "Player";
};
