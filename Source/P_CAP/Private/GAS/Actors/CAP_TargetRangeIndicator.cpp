// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Actors/CAP_TargetRangeIndicator.h"

#include "Components/DecalComponent.h"

ACAP_TargetRangeIndicator::ACAP_TargetRangeIndicator()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Component"));

	RangeDecal = CreateDefaultSubobject<UDecalComponent>("Outline Decal");
	RangeDecal->SetupAttachment(GetRootComponent());
}

void ACAP_TargetRangeIndicator::Initialize(float NewRange)
{
	RangeDecal->DecalSize = FVector(NewRange);
}

