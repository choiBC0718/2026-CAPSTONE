// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Reward/CAP_ChestVisualBase.h"
#include "NiagaraFunctionLibrary.h"


ACAP_ChestVisualBase::ACAP_ChestVisualBase()
{
	PrimaryActorTick.bCanEverTick = false;
	BaseRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BaseRoot"));
	RootComponent = BaseRoot;
}

void ACAP_ChestVisualBase::BeginPlay()
{
	Super::BeginPlay();
	TArray<UStaticMeshComponent*> AllMeshes;
	GetComponents<UStaticMeshComponent>(AllMeshes);
	
	for (UStaticMeshComponent* Mesh : AllMeshes)
	{
		if (Mesh)
		{
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}


void ACAP_ChestVisualBase::PlayOpenAnim()
{
	if (FlashEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), 
			FlashEffect, 
			GetActorLocation(),
			GetActorRotation()
		);
	}
}