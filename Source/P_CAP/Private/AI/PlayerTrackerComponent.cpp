// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/PlayerTrackerComponent.h"
#include "AI/QuadtreeManager.h"              
#include "Kismet/GameplayStatics.h"         
#include "DrawDebugHelpers.h"

void UPlayerTrackerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 맵에서 AQuadtreeManager를 찾아 저장
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AQuadtreeManager::StaticClass());
    
	CachedQuadtreeManager = Cast<AQuadtreeManager>(FoundActor);

	// 0.5초마다 내 위치 기록 타이머
	GetWorld()->GetTimerManager().SetTimer(TrackingTimer, this, &UPlayerTrackerComponent::RecordLocation, 0.5f, true);
}

void UPlayerTrackerComponent::RecordLocation()
{
	if (AActor* OwnerActor = GetOwner())
	{
		FVector CurrentLocation = OwnerActor->GetActorLocation();

		if (bDrawDebug)
			// 플레이어 동선 확인용 초록색 구슬
			DrawDebugSphere(GetWorld(), CurrentLocation, 30.f, 12, FColor::Green, true);

		// 저장해둔 쿼드트리 매니저에게 위치 전달
		if (CachedQuadtreeManager)
		{
			CachedQuadtreeManager->AddVisit(CurrentLocation);
		}
	}  
}