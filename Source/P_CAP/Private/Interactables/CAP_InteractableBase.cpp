// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/CAP_InteractableBase.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InteractionComponent.h"
#include "Components/SphereComponent.h"

ACAP_InteractableBase::ACAP_InteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	InteractionSphere = CreateDefaultSubobject<USphereComponent>("InteractionSphere");
	SetRootComponent(InteractionSphere);
	
	InteractionSphere->SetSphereRadius(130.f);
	InteractionSphere->SetCollisionProfileName(FName("Interactable"));
	InteractionSphere->SetSimulatePhysics(true);
	
	InteractionSphere->BodyInstance.bLockXRotation = true;
	InteractionSphere->BodyInstance.bLockYRotation = true;
	InteractionSphere->BodyInstance.bLockXTranslation = true;
	InteractionSphere->BodyInstance.bLockYTranslation = true;
	InteractionSphere->SetNotifyRigidBodyCollision(true);
}


void ACAP_InteractableBase::BeginPlay()
{
	Super::BeginPlay();
	InteractionSphere->SetSimulatePhysics(true);
	InteractionSphere->OnComponentHit.AddDynamic(this, &ACAP_InteractableBase::OnGroundHit);
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACAP_InteractableBase::OnInteractSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACAP_InteractableBase::OnInteractSphereEndOverlap);
}


void ACAP_InteractableBase::OnGroundHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	InteractionSphere->SetSimulatePhysics(false);
}

void ACAP_InteractableBase::OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(OtherActor))
	{
		if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
			InteractComp->AddInteractable(this);
	}
}

void ACAP_InteractableBase::OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(OtherActor))
	{
		if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
			InteractComp->RemoveInteractable(this);
	}
}