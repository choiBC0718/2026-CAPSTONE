// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CAP_EnemyCharacter.h"

#include "GAS/CAP_AbilitySystemComponent.h"

ACAP_EnemyCharacter::ACAP_EnemyCharacter()
{
}

void ACAP_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponent());
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this,this);
		ASC->InitComponent();
	}
}
