#include "BotPlayController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "StageGoalTrigger.h"
#include "QuadtreeManager.h"
#include "BaseMonster.h"
#include "AnalysisObstacle.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"

ABotPlayController::ABotPlayController()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentState = EBotState::Idle;
	CombatWeight = 0.5f;
	MeleePreference = 0.5f;
	ObstaclePassPreference = 0.5f;
	WaypointsVisited = 0;
	MaxWaypointsBeforeGoal = 5;
	bGoalIsNextTarget = false;
	IdleTimer = 0.f;
	MonsterCheckTimer = 0.f;
	ObstacleCheckTimer = 0.f;
}

void ABotPlayController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GoalActor = Cast<AStageGoalTrigger>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AStageGoalTrigger::StaticClass()));

	CachedQuadtree = Cast<AQuadtreeManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AQuadtreeManager::StaticClass()));

	if (CachedQuadtree)
	{
		MapCenter = CachedQuadtree->GetActorLocation();
		MapExtent = CachedQuadtree->MapSize;
	}
	else
	{
		MapCenter = InPawn->GetActorLocation();
		MapExtent = FVector(2000.f, 2000.f, 100.f);
	}

	RandomizeParameters();

	FTimerHandle StartTimer;
	GetWorld()->GetTimerManager().SetTimer(StartTimer, [this]()
	{
		CurrentState = EBotState::Roaming;
		DecideNextAction();
	}, 1.5f, false);
}

void ABotPlayController::RandomizeParameters()
{
	CombatWeight = FMath::FRandRange(0.0f, 1.0f);
	MeleePreference = FMath::FRandRange(0.0f, 1.0f);
	ObstaclePassPreference = FMath::FRandRange(0.0f, 1.0f);
	MaxWaypointsBeforeGoal = FMath::RandRange(0, 15);

	UE_LOG(LogTemp, Warning, TEXT("봇 파라미터: 전투:%.2f / 근접:%.2f / 돌파:%.2f / 웨이포인트:%d→골"),
		   CombatWeight, MeleePreference, ObstaclePassPreference, MaxWaypointsBeforeGoal);
}

// =============================================
// Tick
// =============================================
void ABotPlayController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EBotState::Idle || CurrentState == EBotState::Finished)
		return;

	if (!GetPawn()) return;

	// Roaming 중일 때만 몬스터/장애물 체크
	if (CurrentState == EBotState::Roaming)
	{
		MonsterCheckTimer += DeltaTime;
		if (MonsterCheckTimer >= 0.5f)
		{
			MonsterCheckTimer = 0.f;
			CheckAndEngageMonsters();
		}

		ObstacleCheckTimer += DeltaTime;
		if (ObstacleCheckTimer >= 0.3f)
		{
			ObstacleCheckTimer = 0.f;
			CheckAndHandleObstacles();
		}
	}

	// Idle 안전장치
	UPathFollowingComponent* PathComp = GetPathFollowingComponent();
	if (PathComp && PathComp->GetStatus() == EPathFollowingStatus::Idle)
	{
		IdleTimer += DeltaTime;
		if (IdleTimer >= 2.0f)
		{
			IdleTimer = 0.f;
			if (CurrentState == EBotState::ApproachingMonster ||
				CurrentState == EBotState::AvoidingObstacle ||
				CurrentState == EBotState::PassingObstacle)
			{
				CurrentState = EBotState::Roaming;
			}
			DecideNextAction();
		}
	}
	else
	{
		IdleTimer = 0.f;
	}
}

// =============================================
// 이동 완료
// =============================================
void ABotPlayController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (CurrentState == EBotState::Finished || CurrentState == EBotState::Idle)
		return;

	// [핵심] 이동이 취소/중단된 경우 무시 → Tick의 Idle 안전장치가 복구
	if (!Result.IsSuccess())
		return;

	// 몬스터 접근 완료 → 공격 → 복귀
	if (CurrentState == EBotState::ApproachingMonster)
	{
		APawn* MyPawn = GetPawn();
		if (MyPawn)
		{
			TArray<AActor*> Overlapping;
			MyPawn->GetOverlappingActors(Overlapping, ABaseMonster::StaticClass());
			if (Overlapping.Num() > 0)
			{
				ABaseMonster* Monster = Cast<ABaseMonster>(Overlapping[0]);
				if (Monster)
				{
					Monster->ReceiveAttack(MyPawn);
					UE_LOG(LogTemp, Warning, TEXT("봇: 접근 후 근접 처치!"));
				}
			}
		}
		CurrentState = EBotState::Roaming;
		DecideNextAction();
		return;
	}

	// 장애물 처리 완료 → 복귀
	if (CurrentState == EBotState::PassingObstacle || CurrentState == EBotState::AvoidingObstacle)
	{
		CurrentState = EBotState::Roaming;
		DecideNextAction();
		return;
	}

	// 일반 웨이포인트 도착 (골 포함)
	if (CurrentState == EBotState::Roaming)
	{
		if (bGoalIsNextTarget)
		{
			// 마지막 웨이포인트(골)에 도착
			// → StageGoalTrigger 오버랩이 자연스럽게 발동됨
			CurrentState = EBotState::Finished;
			UE_LOG(LogTemp, Warning, TEXT("봇: 골(마지막 웨이포인트) 도달!"));
			return;
		}

		WaypointsVisited++;
		DecideNextAction();
	}
}

// =============================================
// 다음 행동 결정
// =============================================
void ABotPlayController::DecideNextAction()
{
	if (WaypointsVisited >= MaxWaypointsBeforeGoal)
		PickGoalAsTarget(); // 마지막 = 골
	else
		PickNewTarget();    // 랜덤/쿼드트리 웨이포인트
}

// =============================================
// 몬스터 전투
// =============================================
void ABotPlayController::CheckAndEngageMonsters()
{
	if (CurrentState != EBotState::Roaming) return;
	if (bGoalIsNextTarget) return; // 골로 향하는 중에는 전투 무시

	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	TArray<AActor*> OverlappingMonsters;
	MyPawn->GetOverlappingActors(OverlappingMonsters, ABaseMonster::StaticClass());
	if (OverlappingMonsters.Num() == 0) return;

	if (FMath::FRand() > CombatWeight) return;

	ABaseMonster* Monster = Cast<ABaseMonster>(OverlappingMonsters[0]);
	if (!Monster) return;

	bool bInInnerZone = Monster->InnerAttackZone->IsOverlappingActor(MyPawn);

	if (bInInnerZone)
	{
		Monster->ReceiveAttack(MyPawn);
		UE_LOG(LogTemp, Warning, TEXT("봇: 근접 처치!"));
	}
	else if (FMath::FRand() < MeleePreference)
	{
		CurrentState = EBotState::ApproachingMonster;
		MoveToActor(Monster, 10.f);
	}
	else
	{
		Monster->ReceiveAttack(MyPawn);
		UE_LOG(LogTemp, Warning, TEXT("봇: 원거리 처치!"));
	}
}

// =============================================
// 장애물 판단
// =============================================
void ABotPlayController::CheckAndHandleObstacles()
{
	if (CurrentState != EBotState::Roaming) return;
	if (bGoalIsNextTarget) return; // 골로 향하는 중에는 장애물 무시

	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	TArray<AActor*> OverlappingObstacles;
	MyPawn->GetOverlappingActors(OverlappingObstacles, AAnalysisObstacle::StaticClass());

	if (OverlappingObstacles.Num() == 0)
	{
		LastHandledObstacle = nullptr;
		return;
	}

	AAnalysisObstacle* Obstacle = Cast<AAnalysisObstacle>(OverlappingObstacles[0]);
	if (!Obstacle) return;

	if (Obstacle == LastHandledObstacle) return;

	LastHandledObstacle = Obstacle;

	if (FMath::FRand() < ObstaclePassPreference)
	{
		CurrentState = EBotState::PassingObstacle;
		MoveToLocation(Obstacle->GetActorLocation(), 10.f);
		UE_LOG(LogTemp, Log, TEXT("봇: 장애물 돌파 시도"));
	}
	else
	{
		CurrentState = EBotState::AvoidingObstacle;
		FVector MyLoc = MyPawn->GetActorLocation();
		FVector AwayDir = (MyLoc - Obstacle->GetActorLocation()).GetSafeNormal();
		FVector AvoidTarget = MyLoc + AwayDir * 500.f;

		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys)
		{
			FNavLocation NavLoc;
			if (NavSys->ProjectPointToNavigation(AvoidTarget, NavLoc, FVector(500.f, 500.f, 200.f)))
			{
				AvoidTarget = NavLoc.Location;
			}
		}

		MoveToLocation(AvoidTarget, 100.f);
		UE_LOG(LogTemp, Log, TEXT("봇: 장애물 회피 시도"));
	}
}

// =============================================
// 웨이포인트: 쿼드트리 기반
// =============================================
void ABotPlayController::PickNewTarget()
{
	if (!GetPawn()) return;

	bGoalIsNextTarget = false;
	FVector Target = GenerateSmartWaypoint();
	EPathFollowingRequestResult::Type Result = MoveToLocation(Target, 100.f);

	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		WaypointsVisited++;
		FTimerHandle NextTimer;
		GetWorld()->GetTimerManager().SetTimer(NextTimer, [this]()
		{
			if (CurrentState != EBotState::Finished)
				DecideNextAction();
		}, 0.1f, false);
	}
	else if (Result == EPathFollowingRequestResult::Failed)
	{
		FTimerHandle RetryTimer;
		GetWorld()->GetTimerManager().SetTimer(RetryTimer, [this]()
		{
			if (CurrentState != EBotState::Finished)
				PickNewTarget();
		}, 1.0f, false);
	}
}

// =============================================
// 마지막 웨이포인트 = 골 위치
// =============================================
void ABotPlayController::PickGoalAsTarget()
{
	if (!GetPawn() || !GoalActor)
	{
		CurrentState = EBotState::Finished;
		return;
	}

	bGoalIsNextTarget = true;
	FVector GoalLoc = GoalActor->GetActorLocation();

	UE_LOG(LogTemp, Warning, TEXT("봇: 웨이포인트 %d개 완료 → 골로 이동 (마지막 웨이포인트)"), WaypointsVisited);

	EPathFollowingRequestResult::Type Result = MoveToLocation(GoalLoc, 5.f);

	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		CurrentState = EBotState::Finished;
		UE_LOG(LogTemp, Warning, TEXT("봇: 이미 골 위치"));
	}
	else if (Result == EPathFollowingRequestResult::Failed)
	{
		// 골로 못 가면 재시도
		bGoalIsNextTarget = false;
		FTimerHandle RetryTimer;
		GetWorld()->GetTimerManager().SetTimer(RetryTimer, [this]()
		{
			if (CurrentState != EBotState::Finished)
				PickGoalAsTarget();
		}, 2.0f, false);
	}
}

FVector ABotPlayController::GenerateSmartWaypoint()
{
	if (!CachedQuadtree)
		return GenerateRandomWaypoint();

	TArray<FQuadtreeNode*> AllLeaves = CachedQuadtree->GetAllLeafNodes();

	TArray<FQuadtreeNode*> UnvisitedNodes;
	for (FQuadtreeNode* Node : AllLeaves)
	{
		if (Node && Node->VisitCount == 0)
			UnvisitedNodes.Add(Node);
	}

	if (UnvisitedNodes.Num() > 0)
	{
		int32 RandIdx = FMath::RandRange(0, UnvisitedNodes.Num() - 1);
		FVector Target = UnvisitedNodes[RandIdx]->Center;

		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys)
		{
			FNavLocation NavLoc;
			if (NavSys->ProjectPointToNavigation(Target, NavLoc, FVector(500.f, 500.f, 200.f)))
			{
				Target = NavLoc.Location;
			}
		}

		return Target;
	}

	return GenerateRandomWaypoint();
}

FVector ABotPlayController::GenerateRandomWaypoint()
{
	float RandX = FMath::FRandRange(-MapExtent.X * 0.9f, MapExtent.X * 0.9f);
	float RandY = FMath::FRandRange(-MapExtent.Y * 0.9f, MapExtent.Y * 0.9f);
	FVector Target = MapCenter + FVector(RandX, RandY, 0.f);

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(Target, NavLoc, FVector(500.f, 500.f, 200.f)))
		{
			Target = NavLoc.Location;
		}
	}

	return Target;
}