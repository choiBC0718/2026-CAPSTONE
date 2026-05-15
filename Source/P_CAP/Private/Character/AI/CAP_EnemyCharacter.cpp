// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CAP_EnemyCharacter.h"

#include "Character/AI/CAP_AIController.h"
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
		ASC->InitComponent(CharacterStatRowName);
	}
}

void ACAP_EnemyCharacter::SetEnemyAIEnabled(bool bEnabled, AActor* TargetActor)
{
	bEnemyAIEnabled = bEnabled;
	CurrentTargetActor = bEnabled ? TargetActor : nullptr;

	ACAP_AIController* CAPAIController = Cast<ACAP_AIController>(GetController());
	if (!CAPAIController)
	{
		return;
	}

	CAPAIController->SetAIEnabled(bEnabled);
	CAPAIController->SetTargetActor(CurrentTargetActor);
}

void ACAP_EnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (bEnemyAIEnabled)
	{
		SetEnemyAIEnabled(true, CurrentTargetActor);
	}
}

void ACAP_EnemyCharacter::OnRoomActivated_Implementation(AActor* TargetActor)
{
	SetEnemyAIEnabled(true, TargetActor);
}

void ACAP_EnemyCharacter::OnRoomDeactivated_Implementation()
{
	SetEnemyAIEnabled(false);
}

void ACAP_EnemyCharacter::OnDead()
{
	SetEnemyAIEnabled(false);
}
