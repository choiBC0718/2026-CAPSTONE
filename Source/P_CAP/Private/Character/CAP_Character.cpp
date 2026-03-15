// Fill out your copyright notice in the Description page of Project Settings.


#include "CAP_Character.h"
#include "Components/SkeletalMeshComponent.h"

ACAP_Character::ACAP_Character()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetupAttachment(GetRootComponent());
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
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

