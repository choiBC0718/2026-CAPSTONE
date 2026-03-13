// Fill out your copyright notice in the Description page of Project Settings.


#include "CAP_Character.h"

// Sets default values
ACAP_Character::ACAP_Character()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetupAttachment(GetRootComponent());
	
	ASC = CreateDefaultSubobject<UCAP_AbilitySystemComponent>("AbilitySystemComponent");
}

void ACAP_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACAP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

