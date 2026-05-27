// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/CAP_WorldNPC.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_DialogueComponent.h"
#include "Components/SphereComponent.h"

ACAP_WorldNPC::ACAP_WorldNPC()
{
	NPCMesh = CreateDefaultSubobject<USkeletalMeshComponent>("NPCMesh");
	NPCMesh->SetupAttachment(InteractionSphere);
	NPCMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NPCMesh->SetRelativeLocation(FVector(0.f,0.f,-130.f));
	NPCMesh->SetRelativeRotation(FRotator(0.0f,-90.f,0.f));

	InteractionSphere->SetSphereRadius(250.f);

	DialogueComponent = CreateDefaultSubobject<UCAP_DialogueComponent>("DialogueComponent");
}

void ACAP_WorldNPC::BeginPlay()
{
	Super::BeginPlay();
	DialogueComponent->OnExecuteSpecialAction.BindUObject(this, &ACAP_WorldNPC::ExecuteSpecialAction);
	DialogueComponent->OnGetSpecialActionCost.BindUObject(this, &ACAP_WorldNPC::GetSpecialActionCost);
}

void ACAP_WorldNPC::Interact(AActor* InsActor, EInteractAction ActionType)
{
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(InsActor))
	{
		if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
			InteractComp->BeginDialogue(DialogueComponent->NPCData);
	}
}

FInteractionPayload ACAP_WorldNPC::GetInteractionPayload() const
{
	FInteractionPayload Payload;
	Payload.ActionData.ShortActionText = TEXT("대화하기");
	
	return Payload;
}

bool ACAP_WorldNPC::GetSpecialActionCost(int& OutCost)
{
	if (InteractionCount == 1)
	{
		OutCost = CostMagicStone;
		return true;
	}
	return false;
}
