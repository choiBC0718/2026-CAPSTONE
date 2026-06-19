// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Reward/CAP_ChestGoldReward.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

ACAP_ChestGoldReward::ACAP_ChestGoldReward()
{
	GoldMesh=CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	GoldMesh->SetupAttachment(InteractionSphere);
	GoldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RotatingMovement=CreateDefaultSubobject<URotatingMovementComponent>("RotatingMovement");
	RotatingMovement->SetUpdatedComponent(InteractionSphere);
	RotatingMovement->RotationRate=FRotator(0.f,45.f,0.f);
}

void ACAP_ChestGoldReward::BeginPlay()
{
	Super::BeginPlay();
}

void ACAP_ChestGoldReward::Interact(AActor* InsActor, EInteractAction ActionType)
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(InsActor);
	UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent();
	if (Player && CurrComp)
	{
		CurrComp->AddCurrency(ECurrencyType::Gold, GoldAmount);
	}
	Destroy();
}

FInteractionPayload ACAP_ChestGoldReward::GetInteractionPayload() const
{
	FInteractionPayload Payload;
	Payload.DetailData = nullptr;
	Payload.ActionData.ShortActionText = FString::Printf(TEXT("+%d 골드"),GoldAmount);
	Payload.ActionData.bShowCurrency=false;
	return Payload;
}
