#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/PlayerBehaviorLearner.h"
#include "MapTestDirector.generated.h"

class UBoxComponent;
class ABaseMonster;
class AAnalysisObstacle;

// Tendency 데이터를 읽어 장애물/몬스터를 동적으로 배치하는 테스트용 디렉터
UCLASS()
class P_CAP_API AMapTestDirector : public AActor
{
	GENERATED_BODY()

public:
	AMapTestDirector();

	// 스폰 범위 (에디터에서 박스 크기로 조절)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Area")
	UBoxComponent* PlayArea;

	// ─── 몬스터 설정 ──────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Monster")
	TSubclassOf<ABaseMonster> MonsterClass;

	// CombatAggression 0.0 → MinMonsterCount, 1.0 → MaxMonsterCount
	UPROPERTY(EditAnywhere, Category = "Monster", meta=(ClampMin="1"))
	int32 MinMonsterCount = 2;

	UPROPERTY(EditAnywhere, Category = "Monster", meta=(ClampMin="1"))
	int32 MaxMonsterCount = 10;

	// MeleePreference 0.0(원거리) → RangedInnerRadius, 1.0(근접) → MeleeInnerRadius
	UPROPERTY(EditAnywhere, Category = "Monster|AttackZone", meta=(ClampMin="50.0"))
	float MeleeInnerRadius = 350.f;

	UPROPERTY(EditAnywhere, Category = "Monster|AttackZone", meta=(ClampMin="50.0"))
	float RangedInnerRadius = 80.f;

	// ─── 장애물 설정 ──────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Obstacle")
	TSubclassOf<AAnalysisObstacle> ObstacleClass;

	// ObstacleBypass 0.0 → MinObstacleCount, 1.0 → MaxObstacleCount
	UPROPERTY(EditAnywhere, Category = "Obstacle", meta=(ClampMin="0"))
	int32 MinObstacleCount = 2;

	UPROPERTY(EditAnywhere, Category = "Obstacle", meta=(ClampMin="0"))
	int32 MaxObstacleCount = 15;

protected:
	virtual void BeginPlay() override;

private:
	void SpawnFromTendency(const FPlayerTendencyModifier& Tendency);
	FVector GetMonsterSpawnLocation(const FVector& Origin, const FVector& Extent, float ExplorationRate);
};
