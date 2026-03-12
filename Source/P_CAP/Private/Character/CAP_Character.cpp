// Fill out your copyright notice in the Description page of Project Settings.


#include "CAP_Character.h"

// Sets default values
ACAP_Character::ACAP_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACAP_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACAP_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACAP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

