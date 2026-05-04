// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/CAP_InteractableBase.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InteractionComponent.h"
#include "Components/SphereComponent.h"
#include "P_CAP/P_CAP.h"

ACAP_InteractableBase::ACAP_InteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootCollision = CreateDefaultSubobject<USphereComponent>("RootCollision");
	SetRootComponent(RootCollision);
	RootCollision->SetSimulatePhysics(true);
	RootCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	RootCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	RootCollision->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Ignore);
	
	InteractionSphere = CreateDefaultSubobject<USphereComponent>("InteractionSphere");
	InteractionSphere->SetupAttachment(RootCollision);
	InteractionSphere->SetSphereRadius(100.f);
	InteractionSphere->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
}


void ACAP_InteractableBase::BeginPlay()
{
	Super::BeginPlay();

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACAP_InteractableBase::OnInteractSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACAP_InteractableBase::OnInteractSphereEndOverlap);
	RootCollision->OnComponentHit.AddDynamic(this, &ACAP_InteractableBase::OnRootCollisionHit);
}


void ACAP_InteractableBase::DropItem()
{
	if (RootCollision)
	{
		FVector DropImpulse = FVector(0.f,0.f, 600.f);
		RootCollision->AddImpulse(DropImpulse, NAME_None, true);
	}
}

void ACAP_InteractableBase::OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	/*
	ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->GetInventoryComponent()->SetNearbyInteractable(this);
		PlayerCharacter->UpdateInteractUI(true);
	}
	*/
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(OtherActor))
	{
		if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
			InteractComp->SetNearbyInteractable(this);
	}
}

void ACAP_InteractableBase::OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	/*
	ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->GetInventoryComponent()->SetNearbyInteractable(nullptr);
		PlayerCharacter->UpdateInteractUI(false);
	}
	*/
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(OtherActor))
	{
		if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
			if (InteractComp->GetNearbyInteractable() == this)
				InteractComp->SetNearbyInteractable(nullptr);
	}
}

void ACAP_InteractableBase::OnRootCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	RootCollision->SetSimulatePhysics(false);
}
