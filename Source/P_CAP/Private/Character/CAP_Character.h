// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CAP_Character.generated.h"

UCLASS()
class ACAP_Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACAP_Character();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/**		Components		**/
	UPROPERTY(VisibleAnywhere, Category="Ability")
	class UCAP_AbilitySystemComponent* CAPAbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, Category="Ability")
	class UCAP_AttributeSet* CAPAttributeSet;

protected:
	UCAP_AbilitySystemComponent* GetAbilitySystemComponent() const;
};
