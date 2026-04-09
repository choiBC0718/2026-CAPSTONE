// Fill out your copyright notice in the Description page of Project Settings.

#include "AnalysisObstacle.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "AI/PlayerTrackerComponent.h" // 트래커 컴포넌트 접근을 위해 인클루드

AAnalysisObstacle::AAnalysisObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	// OuterZone 설정 (감지 반경)
	OuterZone = CreateDefaultSubobject<UBoxComponent>(TEXT("OuterZone"));
	RootComponent = OuterZone;
	OuterZone->SetBoxExtent(FVector(300.f, 300.f, 200.f));
	OuterZone->SetCollisionProfileName(TEXT("Trigger"));

	// InnerZone 설정 (실제 돌파 판정 반경)
	InnerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InnerZone"));
	InnerZone->SetupAttachment(RootComponent);
	InnerZone->SetBoxExtent(FVector(100.f, 100.f, 200.f));
	InnerZone->SetCollisionProfileName(TEXT("Trigger"));

	bHasPassedThrough = false;
	bIsTracking = false;
}

void AAnalysisObstacle::BeginPlay()
{
	Super::BeginPlay();

	// 오버랩 이벤트 바인딩
	OuterZone->OnComponentBeginOverlap.AddDynamic(this, &AAnalysisObstacle::OnOuterOverlapBegin);
	OuterZone->OnComponentEndOverlap.AddDynamic(this, &AAnalysisObstacle::OnOuterOverlapEnd);
	InnerZone->OnComponentBeginOverlap.AddDynamic(this, &AAnalysisObstacle::OnInnerOverlapBegin);
}

void AAnalysisObstacle::OnOuterOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 플레이어가 Outer 구역에 진입하면 상태 추적 시작
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		bIsTracking = true;
		bHasPassedThrough = false; 
	}
}

void AAnalysisObstacle::OnInnerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 추적 중인 플레이어가 Inner 구역(중심)을 밟았을 때
	if (bIsTracking && OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		bHasPassedThrough = true; // 돌파 완료 플래그 활성화
		
		UPlayerTrackerComponent* Tracker = OtherActor->FindComponentByClass<UPlayerTrackerComponent>();
		if (Tracker)
		{
			Tracker->PassedObstacleCount++;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("장애물 돌파(PassThrough) 감지"));
	}
}

void AAnalysisObstacle::OnOuterOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 플레이어가 Outer 구역을 완전히 빠져나갔을 때 최종 판정
	if (bIsTracking && OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		// 중심을 밟지 않고 나갔다면 회피(Avoid)로 판정
		if (!bHasPassedThrough)
		{
			UPlayerTrackerComponent* Tracker = OtherActor->FindComponentByClass<UPlayerTrackerComponent>();
			if (Tracker)
			{
				Tracker->AvoidedObstacleCount++;
			}
			UE_LOG(LogTemp, Warning, TEXT("장애물 회피(Avoid) 감지"));
		}
		
		// 다음 측정을 위해 상태 초기화
		bIsTracking = false;
		bHasPassedThrough = false;
	}
}