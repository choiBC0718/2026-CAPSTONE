// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CAP_Character.h"

#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AttributeSet.h"

ACAP_Character::ACAP_Character()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetupAttachment(GetRootComponent());
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CAPAbilitySystemComponent = CreateDefaultSubobject<UCAP_AbilitySystemComponent>("Ability System Component");
	CAPAttributeSet = CreateDefaultSubobject<UCAP_AttributeSet>("Attribute Set");
}

const TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>>& ACAP_Character::GetAbilities() const
{
	return CAPAbilitySystemComponent->GetAbilities();
}


UAbilitySystemComponent* ACAP_Character::GetAbilitySystemComponent() const
{
	return CAPAbilitySystemComponent;
}

