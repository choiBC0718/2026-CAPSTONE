// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CAP_WeaponBase.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/SphereComponent.h"

ACAP_WeaponBase::ACAP_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>("Interaction Collision Component");
	SetRootComponent(InteractionSphere);
}


void ACAP_WeaponBase::BeginPlay()
{
	Super::BeginPlay();

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACAP_WeaponBase::OnInteractSphereOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACAP_WeaponBase::OnInteractSphereEndOverlap);
}

void ACAP_WeaponBase::InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter && WeaponDA)
	{
		PlayerCharacter->PickupWeapon(WeaponDA);
		UE_LOG(LogTemp, Warning, TEXT("아이템 장착"));
		Destroy();
	}
}

void ACAP_WeaponBase::InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("아이템 분해"));
	Destroy();
}

void ACAP_WeaponBase::OnInteractSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->SetNearbyInteractable(this);
		PlayerCharacter->UpdateInteractUI(true);
	}
}

void ACAP_WeaponBase::OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->SetNearbyInteractable(nullptr);
		PlayerCharacter->UpdateInteractUI(false);
	}
}

