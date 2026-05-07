#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BotPlayController.generated.h"

class ABaseMonster;
class AStageGoalTrigger;
class AQuadtreeManager;
class AAnalysisObstacle;

enum class EBotState : uint8
{
	Idle,
	Roaming,
	ApproachingMonster,
	AvoidingObstacle,
	PassingObstacle,
	Finished
};

UCLASS()
class P_CAP_API ABotPlayController : public AAIController
{
	GENERATED_BODY()

public:
	ABotPlayController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	EBotState CurrentState;

	// 랜덤 행동 파라미터
	float CombatWeight;
	float MeleePreference;
	float ObstaclePassPreference;
	int32 MaxWaypointsBeforeGoal; // 이만큼 돌아다닌 후 마지막에 골로 감
	int32 WaypointsVisited;
	bool bGoalIsNextTarget;       // 현재 이동 중인 목표가 골인지

	// 맵 정보
	FVector MapCenter;
	FVector MapExtent;

	UPROPERTY()
	AStageGoalTrigger* GoalActor;

	UPROPERTY()
	AQuadtreeManager* CachedQuadtree;

	UPROPERTY()
	AAnalysisObstacle* LastHandledObstacle;

	float IdleTimer;
	float MonsterCheckTimer;
	float ObstacleCheckTimer;

	void RandomizeParameters();
	void DecideNextAction();
	void PickNewTarget();          // 랜덤/쿼드트리 기반 웨이포인트
	void PickGoalAsTarget();       // 마지막 웨이포인트 = 골 위치
	void CheckAndEngageMonsters();
	void CheckAndHandleObstacles();
	FVector GenerateSmartWaypoint();
	FVector GenerateRandomWaypoint();
};