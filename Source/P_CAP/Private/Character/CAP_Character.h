// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "CAP_Character.generated.h"

UCLASS()
class ACAP_Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACAP_Character();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual UCAP_AbilitySystemComponent* GetAbilitySystemComponent() const {return ASC;}
protected:
	virtual void BeginPlay() override;


private:
	/**		컴포넌트		**/
	UPROPERTY(VisibleDefaultsOnly, Category="Gameplay Ability")
	class UCAP_AbilitySystemComponent* ASC;
};
