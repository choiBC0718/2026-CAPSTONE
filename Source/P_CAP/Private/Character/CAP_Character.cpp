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



void ACAP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UCAP_AbilitySystemComponent* ACAP_Character::GetAbilitySystemComponent() const
{
	return CAPAbilitySystemComponent;
}

