// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/PlayerTrackerComponent.h"
#include "AI/QuadtreeManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UPlayerTrackerComponent::UPlayerTrackerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerTrackerComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AQuadtreeManager::StaticClass());
	CachedQuadtreeManager = Cast<AQuadtreeManager>(FoundActor);

	GetWorld()->GetTimerManager().SetTimer(TrackingTimer, this, &UPlayerTrackerComponent::RecordLocation, 0.5f, true);
}

void UPlayerTrackerComponent::RecordLocation()
{
	if (AActor* OwnerActor = GetOwner())
	{
		FVector CurrentLocation = OwnerActor->GetActorLocation();
		//DrawDebugSphere(GetWorld(), CurrentLocation, 30.f, 12, FColor::Green, true);

		if (CachedQuadtreeManager)
		{
			CachedQuadtreeManager->AddVisit(CurrentLocation);
		}
	}
}
