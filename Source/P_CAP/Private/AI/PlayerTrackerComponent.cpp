// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/PlayerTrackerComponent.h"
#include "AI/QuadtreeManager.h"              
#include "Kismet/GameplayStatics.h"
#include "BaseMonster.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"

// [추가된 부분] 링커 에러를 해결하기 위한 생성자 구현부
UPlayerTrackerComponent::UPlayerTrackerComponent()
{
	// 우리는 위치 기록을 위해 매 프레임(Tick) 대신 타이머(Timer)를 사용하므로, 
	// 성능 최적화를 위해 컴포넌트 틱 기능은 꺼둡니다.
	PrimaryComponentTick.bCanEverTick = false;
}

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
		//DrawDebugSphere(GetWorld(), CurrentLocation, 30.f, 12, FColor::Green, true);

		if (CachedQuadtreeManager)
		{
			CachedQuadtreeManager->AddVisit(CurrentLocation);
		}
		
		// 내 주변에 몬스터가 있는지 확인하고 텍스트 띄우기
		TArray<AActor*> OverlappingMonsters;
		OwnerActor->GetOverlappingActors(OverlappingMonsters, ABaseMonster::StaticClass());
       
		if (OverlappingMonsters.Num() > 0)
		{
			if (ABaseMonster* Monster = Cast<ABaseMonster>(OverlappingMonsters[0]))
			{
				if (Monster->InnerAttackZone->IsOverlappingActor(OwnerActor))
				{
					// 1번 키 슬롯에 붉은색 텍스트 띄우기 (크기 2배)
					GEngine->AddOnScreenDebugMessage(1, 0.6f, FColor::Red, TEXT("F 처치 가능 (Inner - 근접)"), true, FVector2D(2.0f, 2.0f));
				}
				else if (Monster->OuterAttackZone->IsOverlappingActor(OwnerActor))
				{
					// 1번 키 슬롯에 노란색 텍스트 띄우기
					GEngine->AddOnScreenDebugMessage(1, 0.6f, FColor::Yellow, TEXT("F 처치 가능 (Outer - 원거리)"), true, FVector2D(2.0f, 2.0f));
				}
			}
		}
		// ========================================================
	}  
}