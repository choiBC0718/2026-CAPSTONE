#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerTrackerComponent.generated.h"

// 쿼드트리 매니저를 인식하기 위한 전방 선언
class AQuadtreeManager; 

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class P_CAP_API UPlayerTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerTrackerComponent();

	// ----------------------------------------------------
	// [복구됨: 기존 시스템] 쿼드트리 위치 기록용 변수 및 함수
	// ----------------------------------------------------
	UPROPERTY()
	AQuadtreeManager* CachedQuadtreeManager;

	FTimerHandle TrackingTimer;

	UFUNCTION()
	void RecordLocation();

	// ----------------------------------------------------
	// [Z축 데이터] 장애물 돌파/회피 카운트
	// ----------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tracking|Obstacle")
	int32 PassedObstacleCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tracking|Obstacle")
	int32 AvoidedObstacleCount = 0;

	// ----------------------------------------------------
	// [W축 데이터] 몬스터 스폰/처치 카운트
	// ----------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tracking|Monster")
	int32 TotalSpawnedMonsters = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tracking|Monster")
	int32 KilledMonsterCount = 0;

	// 몬스터가 죽을 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Tracking|Monster")
	void AddMonsterKill() 
	{ 
		KilledMonsterCount++; 
	}

	// ----------------------------------------------------
	// [전투 성향 데이터] 근접 vs 원거리 처치 비율
	// ----------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tracking|Combat")
	int32 MeleeKillCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tracking|Combat")
	int32 RangedKillCount = 0;

protected:
	virtual void BeginPlay() override;
};