// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/CAP_WorldNPC.h"

#include "Components/SphereComponent.h"

ACAP_WorldNPC::ACAP_WorldNPC()
{
	NPCMesh = CreateDefaultSubobject<USkeletalMeshComponent>("NPCMesh");
	NPCMesh->SetupAttachment(InteractionSphere);
	NPCMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACAP_WorldNPC::Interact(AActor* InsActor, EInteractAction ActionType)
{
	
}

FInteractionPayload ACAP_WorldNPC::GetInteractionPayload() const
{
	FInteractionPayload Payload;
	Payload.ActionData.ShortActionText = TEXT("대화하기");
	Payload.ActionData.bShowCurrency = false; 
	Payload.DetailData = nullptr;
	
	return Payload;
}
