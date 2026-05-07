#include "AI/MapTestDirector.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/PlayerBehaviorLearner.h"
#include "AI/BaseMonster.h"
#include "AI/AnalysisObstacle.h"

AMapTestDirector::AMapTestDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	PlayArea = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayArea"));
	RootComponent = PlayArea;
	PlayArea->SetBoxExtent(FVector(2000.f, 2000.f, 200.f));
	PlayArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMapTestDirector::BeginPlay()
{
	Super::BeginPlay();

	// 플레이어/봇 초기화 대기 후 실행
	FTimerHandle InitTimer;
	GetWorld()->GetTimerManager().SetTimer(InitTimer, [this]()
	{
		FPlayerTendencyModifier Tendency; // 기본값 0.5

		APlayerBehaviorLearner* Learner = Cast<APlayerBehaviorLearner>(
			UGameplayStatics::GetActorOfClass(GetWorld(), APlayerBehaviorLearner::StaticClass()));

		if (Learner)
		{
			Tendency = Learner->GetCurrentPlayerTendency();
			UE_LOG(LogTemp, Warning, TEXT("[MapTestDirector] Tendency 로드됨: 탐색=%.2f 전투=%.2f 근접=%.2f 장애물=%.2f"),
				Tendency.ExplorationRate, Tendency.CombatAggression,
				Tendency.MeleePreference, Tendency.ObstacleBypass);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[MapTestDirector] Learner 없음 → 기본값(0.5)으로 배치"));
		}

		SpawnFromTendency(Tendency);
	}, 1.0f, false);
}

void AMapTestDirector::SpawnFromTendency(const FPlayerTendencyModifier& Tendency)
{
	const FVector Origin = PlayArea->Bounds.Origin;
	const FVector Extent = PlayArea->Bounds.BoxExtent;

	// ─── 장애물 배치 ──────────────────────────────────────────
	// ObstacleBypass가 높을수록 장애물을 잘 통과한다 → 더 많이 배치해 도전 유지
	const int32 ObstacleCount = FMath::RoundToInt(
		FMath::Lerp((float)MinObstacleCount, (float)MaxObstacleCount, Tendency.ObstacleBypass));

	if (ObstacleClass)
	{
		for (int32 i = 0; i < ObstacleCount; i++)
		{
			const float RandX = FMath::RandRange(-Extent.X * 0.8f, Extent.X * 0.8f);
			const float RandY = FMath::RandRange(-Extent.Y * 0.8f, Extent.Y * 0.8f);
			GetWorld()->SpawnActor<AAnalysisObstacle>(ObstacleClass, Origin + FVector(RandX, RandY, 0.f), FRotator::ZeroRotator);
		}
	}

	// ─── 몬스터 배치 ──────────────────────────────────────────
	// CombatAggression이 높을수록 전투를 즐긴다 → 몬스터 많이 배치
	const int32 MonsterCount = FMath::RoundToInt(
		FMath::Lerp((float)MinMonsterCount, (float)MaxMonsterCount, Tendency.CombatAggression));

	// MeleePreference: 1.0(근접 선호) → InnerZone 크게 / 0.0(원거리 선호) → InnerZone 작게
	const float InnerRadius = FMath::Lerp(RangedInnerRadius, MeleeInnerRadius, Tendency.MeleePreference);

	if (MonsterClass)
	{
		for (int32 i = 0; i < MonsterCount; i++)
		{
			const FVector SpawnLoc = GetMonsterSpawnLocation(Origin, Extent, Tendency.ExplorationRate);
			ABaseMonster* Monster = GetWorld()->SpawnActor<ABaseMonster>(MonsterClass, SpawnLoc, FRotator::ZeroRotator);

			if (Monster)
			{
				Monster->InnerAttackRadius = InnerRadius;
				if (Monster->InnerAttackZone)
				{
					Monster->InnerAttackZone->SetSphereRadius(InnerRadius);
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=== [MapTestDirector] 배치 완료 ==="));
	UE_LOG(LogTemp, Warning, TEXT("  장애물 %d개  (ObstacleBypass=%.2f)"), ObstacleCount, Tendency.ObstacleBypass);
	UE_LOG(LogTemp, Warning, TEXT("  몬스터 %d마리 (CombatAggression=%.2f)"), MonsterCount, Tendency.CombatAggression);
	UE_LOG(LogTemp, Warning, TEXT("  InnerZone 반경 %.0f  (MeleePreference=%.2f)"), InnerRadius, Tendency.MeleePreference);
}

FVector AMapTestDirector::GetMonsterSpawnLocation(const FVector& Origin, const FVector& Extent, float ExplorationRate)
{
	// ExplorationRate 높음(탐험형) → 외곽 배치, 낮음(직진형) → 중앙 배치
	if (FMath::FRand() < ExplorationRate)
	{
		const bool bXAxis = FMath::RandBool();
		const float Sign = FMath::RandBool() ? 1.f : -1.f;

		const float RandX = bXAxis
			? FMath::RandRange(Extent.X * 0.6f, Extent.X * 0.9f) * Sign
			: FMath::RandRange(-Extent.X * 0.9f, Extent.X * 0.9f);
		const float RandY = bXAxis
			? FMath::RandRange(-Extent.Y * 0.9f, Extent.Y * 0.9f)
			: FMath::RandRange(Extent.Y * 0.6f, Extent.Y * 0.9f) * Sign;

		return Origin + FVector(RandX, RandY, 0.f);
	}
	else
	{
		const float RandX = FMath::RandRange(-Extent.X * 0.3f, Extent.X * 0.3f);
		const float RandY = FMath::RandRange(-Extent.Y * 0.3f, Extent.Y * 0.3f);
		return Origin + FVector(RandX, RandY, 0.f);
	}
}
