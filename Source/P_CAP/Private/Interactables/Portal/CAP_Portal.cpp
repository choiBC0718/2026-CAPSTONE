// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Portal/CAP_Portal.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ACAP_Portal::ACAP_Portal()
{
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>("PortalMesh");
	PortalMesh->SetupAttachment(InteractionSphere);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortalMesh->SetRelativeLocation(FVector(0.f,0.f,-130.f));
	PortalMesh->SetRelativeRotation(FRotator(0.0f,-90.f,0.f));
}

void ACAP_Portal::Interact(AActor* InsActor, EInteractAction ActionType)
{
	Super::Interact(InsActor, ActionType);

	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(InsActor))
		Player->SaveProgressionBeforeChangeLevel();
	
	UGameplayStatics::OpenLevel(this,NextStageName);
}

FInteractionPayload ACAP_Portal::GetInteractionPayload() const
{
	FInteractionPayload Payload;
	Payload.ActionData.ShortActionText = TEXT("스테이지 진입");
	
	return Payload;
}
