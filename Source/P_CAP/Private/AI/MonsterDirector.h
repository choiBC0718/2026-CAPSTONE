#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/PlayerBehaviorLearner.h" 
#include "MonsterDirector.generated.h"

class UBoxComponent;

UCLASS()
class P_CAP_API AMonsterDirector : public AActor
{
	GENERATED_BODY()

public:
	AMonsterDirector();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Area")
	UBoxComponent* SpawnVolume;

	UPROPERTY(EditAnywhere, Category = "Spawn Settings")
	TSubclassOf<ACharacter> MonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn Settings", meta=(ClampMin="1"))
	int32 MonsterCount = 10;

	// 몇 초마다 죽은 몬스터를 보충 스폰할지 (0 = 리스폰 없음)
	UPROPERTY(EditAnywhere, Category = "Spawn Settings", meta=(ClampMin="0"))
	float RespawnInterval = 10.f;

	// =============================================
	// [추가] 스폰 패턴 튜닝 파라미터
	// =============================================

	// SpeedRunner 패턴: 맵 중앙 몇 %에 스폰할지 (0.2 = 중앙 20%)
	UPROPERTY(EditAnywhere, Category = "Spawn Settings|Pattern", meta=(ClampMin="0.05", ClampMax="0.5"))
	float SpeedRunnerCenterRatio = 0.2f;

	// Explorer 패턴: 가장자리 시작 지점 (0.7 = 70% 지점부터 끝까지)
	UPROPERTY(EditAnywhere, Category = "Spawn Settings|Pattern", meta=(ClampMin="0.3", ClampMax="0.95"))
	float ExplorerEdgeStartRatio = 0.7f;

	// CombatAggression=0.0 일 때 MonsterCount 배율 (최소)
	UPROPERTY(EditAnywhere, Category = "Spawn Settings|Pattern", meta=(ClampMin="0.1", ClampMax="1.0"))
	float MinMonsterMultiplier = 0.5f;

	// CombatAggression=1.0 일 때 MonsterCount 배율 (최대)
	UPROPERTY(EditAnywhere, Category = "Spawn Settings|Pattern", meta=(ClampMin="1.0", ClampMax="3.0"))
	float MaxMonsterMultiplier = 1.5f;

	UFUNCTION(BlueprintCallable, Category = "Spawn Logic")
	void SpawnMonstersByTendency(const FPlayerTendencyModifier& PlayerTendency);

protected:
	virtual void BeginPlay() override;

private:
	// 살아있는 몬스터 약한 포인터 목록 (리스폰 계산용)
	TArray<TWeakObjectPtr<ACharacter>> SpawnedMonsters;

	// CombatAggression 기반으로 계산된 실제 스폰 목표 수 (리스폰 시 재사용)
	int32 CachedDynamicMonsterCount = 0;

	FTimerHandle RespawnTimerHandle;

	ACharacter* SpawnOneMonster(const FPlayerTendencyModifier& Tendency);
	void CheckAndRespawn();

	FVector GetSpeedRunnerSpawnPoint(FVector Center, FVector Extent);
	FVector GetExplorerSpawnPoint(FVector Center, FVector Extent);
};