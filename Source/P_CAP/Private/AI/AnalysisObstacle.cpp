#include "AnalysisObstacle.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "AI/PlayerTrackerComponent.h"

AAnalysisObstacle::AAnalysisObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트: 메시 (순수 비주얼 — 충돌 없음)
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 감지 존들은 메시에 붙임
	OuterZone = CreateDefaultSubobject<UBoxComponent>(TEXT("OuterZone"));
	OuterZone->SetupAttachment(RootComponent);
	OuterZone->SetCollisionProfileName(TEXT("Trigger"));

	InnerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InnerZone"));
	InnerZone->SetupAttachment(RootComponent);
	InnerZone->SetCollisionProfileName(TEXT("Trigger"));

	bHasPassedThrough = false;
	bIsTracking = false;
	bMonsterSpawned = false;
}

void AAnalysisObstacle::BeginPlay()
{
	Super::BeginPlay();

	// Blueprint 덮어쓰기 방지 — 메시는 완전 비충돌, Zone들은 순수 Overlap Trigger
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OuterZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OuterZone->SetCollisionResponseToAllChannels(ECR_Overlap);

	InnerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InnerZone->SetCollisionResponseToAllChannels(ECR_Overlap);

	OuterZone->SetBoxExtent(OuterZoneExtent);
	InnerZone->SetBoxExtent(InnerZoneExtent);

	OuterZone->OnComponentBeginOverlap.AddDynamic(this, &AAnalysisObstacle::OnOuterOverlapBegin);
	OuterZone->OnComponentEndOverlap.AddDynamic(this, &AAnalysisObstacle::OnOuterOverlapEnd);
	InnerZone->OnComponentBeginOverlap.AddDynamic(this, &AAnalysisObstacle::OnInnerOverlapBegin);
}

void AAnalysisObstacle::OnOuterOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->FindComponentByClass<UPlayerTrackerComponent>())
	{
		bIsTracking = true;
		bHasPassedThrough = false; 
	}
}

void AAnalysisObstacle::OnInnerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsTracking && OtherActor && OtherActor->FindComponentByClass<UPlayerTrackerComponent>())
	{
		bHasPassedThrough = true;

		UPlayerTrackerComponent* Tracker = OtherActor->FindComponentByClass<UPlayerTrackerComponent>();
		if (Tracker)
		{
			Tracker->PassedObstacleCount++;
		}

		// 돌파 시 몬스터 소환 — 장애물당 한 번만
		if (!bMonsterSpawned && BypassMonsterClass)
		{
			bMonsterSpawned = true;

			UWorld* World = GetWorld();
			if (World)
			{
				// 장애물 중심에서 랜덤 수평 방향으로 BypassSpawnRadius 거리에 소환
				const float Angle = FMath::FRandRange(0.f, 360.f);
				const FVector Offset = FVector(
					FMath::Cos(FMath::DegreesToRadians(Angle)) * BypassSpawnRadius,
					FMath::Sin(FMath::DegreesToRadians(Angle)) * BypassSpawnRadius,
					0.f);
				const FVector SpawnLocation = GetActorLocation() + Offset;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				ACharacter* Spawned = World->SpawnActor<ACharacter>(BypassMonsterClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				if (Spawned)
				{
					Spawned->SpawnDefaultController();
					UE_LOG(LogTemp, Warning, TEXT("장애물 돌파 → 몬스터 소환: %s"), *Spawned->GetName());
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("장애물 돌파(PassThrough) 감지"));
	}
}

void AAnalysisObstacle::OnOuterOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bIsTracking && OtherActor && OtherActor->FindComponentByClass<UPlayerTrackerComponent>())
	{
		if (!bHasPassedThrough)
		{
			UPlayerTrackerComponent* Tracker = OtherActor->FindComponentByClass<UPlayerTrackerComponent>();
			if (Tracker)
			{
				Tracker->AvoidedObstacleCount++;
			}
			UE_LOG(LogTemp, Warning, TEXT("장애물 회피(Avoid) 감지"));
		}
		
		bIsTracking = false;
		bHasPassedThrough = false;
	}
}