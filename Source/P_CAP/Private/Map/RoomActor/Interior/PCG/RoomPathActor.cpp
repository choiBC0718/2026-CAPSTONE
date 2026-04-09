// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/PCG/RoomPathActor.h"

ARoomPathActor::ARoomPathActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	PathSpline->SetupAttachment(Root);
	PathSpline->SetClosedLoop(false);
	PathSpline->SetDrawDebug(true);
}

void ARoomPathActor::InitializePath(const TArray<FVector>& InWorldPathPoints)
{
	if (!PathSpline)
	{
		return;
	}

	PathSpline->ClearSplinePoints(false);

	for (const FVector& WorldPoint : InWorldPathPoints)
	{
		PathSpline->AddSplinePoint(WorldPoint, ESplineCoordinateSpace::World, false);
	}

	PathSpline->UpdateSpline();
}
