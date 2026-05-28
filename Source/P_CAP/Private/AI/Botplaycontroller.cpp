#include "BotPlayController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "StageGoalTrigger.h"
#include "QuadtreeManager.h"
#include "AnalysisObstacle.h"
#include "AI/PlayerTrackerComponent.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h"

// 600u 이내에서 가장 가까운 몬스터 탐색 (TActorIterator 거리 기반)
static ACharacter* FindNearestMonster(APawn* MyPawn, float Radius = 600.f)
{
	ACharacter* Nearest = nullptr;
	float MinDist = Radius;
	for (TActorIterator<ACharacter> It(MyPawn->GetWorld()); It; ++It)
	{
		ACharacter* C = *It;
		if (C == MyPawn) continue;
		if (C->FindComponentByClass<UPlayerTrackerComponent>()) continue;
		float Dist = FVector::Dist(MyPawn->GetActorLocation(), C->GetActorLocation());
		if (Dist < MinDist) { MinDist = Dist; Nearest = C; }
	}
	return Nearest;
}

// 600u 이내에서 가장 가까운 장애물 탐색
static AAnalysisObstacle* FindNearestObstacle(APawn* MyPawn, float Radius = 600.f)
{
	AAnalysisObstacle* Nearest = nullptr;
	float MinDist = Radius;
	for (TActorIterator<AAnalysisObstacle> It(MyPawn->GetWorld()); It; ++It)
	{
		AAnalysisObstacle* Obs = *It;
		float Dist = FVector::Dist(MyPawn->GetActorLocation(), Obs->GetActorLocation());
		if (Dist < MinDist) { MinDist = Dist; Nearest = Obs; }
	}
	return Nearest;
}

static void KillMonster(APawn* MyPawn, ACharacter* Monster, bool bMelee)
{
	if (!IsValid(Monster)) return;

	UPlayerTrackerComponent* Tracker = MyPawn->FindComponentByClass<UPlayerTrackerComponent>();
	if (Tracker)
	{
		if (bMelee) Tracker->MeleeKillCount++;
		else        Tracker->RangedKillCount++;
		Tracker->AddMonsterKill();
	}

	// GAS 사망 플로우(OnDead, 사망 애니메이션 등)가 타도록 ApplyDamage 먼저 시도
	UGameplayStatics::ApplyDamage(Monster, 99999.f, nullptr, MyPawn, UDamageType::StaticClass());

	// GAS가 없거나 즉사 처리가 안 됐을 경우 폴백
	if (IsValid(Monster))
		Monster->Destroy();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, bMelee ? FColor::Orange : FColor::Cyan,
			FString::Printf(TEXT("봇: %s 처치!"), bMelee ? TEXT("근접") : TEXT("원거리")));
	}
}

ABotPlayController::ABotPlayController()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentState = EBotState::Idle;
	CombatWeight = 0.5f;
	MeleePreference = 0.5f;
	ObstaclePassPreference = 0.5f;
	ExplorationPreference = 0.5f;
	WaypointsVisited = 0;
	MaxWaypointsBeforeGoal = 5;
	bGoalIsNextTarget = false;
	IdleTimer = 0.f;
	MonsterCheckTimer = 0.f;
	ObstacleCheckTimer = 0.f;
	LastHandledMonster = nullptr;
	bIsRangedAttack = false;
	RunTimer = 0.f;
	MaxRunTime = 90.f;
	bReloadScheduled = false;
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
	CombatWeight           = FMath::FRandRange(0.0f, 1.0f);
	MeleePreference        = FMath::FRandRange(0.0f, 1.0f);
	ObstaclePassPreference = FMath::FRandRange(0.0f, 1.0f);
	ExplorationPreference  = FMath::FRandRange(0.0f, 1.0f);

	// ExplorationPreference: 0=스피드런(웨이포인트 0개) / 1=탐험(웨이포인트 15개)
	MaxWaypointsBeforeGoal = FMath::RoundToInt(FMath::Lerp(0.f, 15.f, ExplorationPreference));

	UE_LOG(LogTemp, Warning,
		TEXT("봇 파라미터: 전투:%.2f / 근접:%.2f / 돌파:%.2f / 탐험:%.2f / 웨이포인트:%d→골"),
		CombatWeight, MeleePreference, ObstaclePassPreference, ExplorationPreference, MaxWaypointsBeforeGoal);
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

	// 실제 시간 기준 런 타임 체크 (배속 영향 없음)
	float RealDelta = DeltaTime / FMath::Max(GetWorld()->GetWorldSettings()->TimeDilation, 0.01f);
	RunTimer += RealDelta;
	if (RunTimer >= MaxRunTime && !bGoalIsNextTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("봇: 시간 초과(%.0fs) → 강제 골 이동"), MaxRunTime);
		CurrentState = EBotState::Roaming;
		PickGoalAsTarget();
		return;
	}

	// 골로 이동 중일 때 거리 직접 체크 (NavMesh/PathFollowing 우회)
	if (bGoalIsNextTarget && !bReloadScheduled && GoalActor && GetPawn())
	{
		float DistToGoal = FVector::Dist(GetPawn()->GetActorLocation(), GoalActor->GetActorLocation());
		if (DistToGoal < 300.f)
		{
			bReloadScheduled = true;
			CurrentState = EBotState::Finished;
			UE_LOG(LogTemp, Warning, TEXT("봇: 골 근접(%.0fu) → 데이터 정산 후 리로드"), DistToGoal);
			GoalActor->ProcessGoalForActor(GetPawn());
			FString LevelName = GetWorld()->GetName();
			TWeakObjectPtr<ABotPlayController> WeakThis(this);
			GetWorldTimerManager().SetTimer(ReloadTimerHandle, [WeakThis, LevelName]()
			{
				if (WeakThis.IsValid())
					UGameplayStatics::OpenLevel(WeakThis.Get(), FName(*LevelName));
			}, 1.5f, false);
			return;
		}
	}

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
				CurrentState == EBotState::AvoidingMonster   ||
				CurrentState == EBotState::AvoidingObstacle  ||
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

	// 몬스터 접근(or 원거리 대기) 완료 → 처치 → 복귀
	if (CurrentState == EBotState::ApproachingMonster)
	{
		APawn* MyPawn = GetPawn();
		if (MyPawn)
		{
			ACharacter* Monster = FindNearestMonster(MyPawn, 800.f);
			if (Monster)
			{
				bool bMelee = !bIsRangedAttack;
				KillMonster(MyPawn, Monster, bMelee);
				LastHandledMonster = nullptr;
				UE_LOG(LogTemp, Warning, TEXT("봇: %s 처치!"), bMelee ? TEXT("근접") : TEXT("원거리"));
			}
		}
		bIsRangedAttack = false;
		CurrentState = EBotState::Roaming;
		DecideNextAction();
		return;
	}

	// 몬스터 회피 완료 → 복귀
	if (CurrentState == EBotState::AvoidingMonster)
	{
		LastHandledMonster = nullptr;
		CurrentState = EBotState::Roaming;
		DecideNextAction();
		return;
	}

	// 장애물 회피 완료 → 복귀 (웨이포인트 카운트에 포함하지 않음)
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
			if (bReloadScheduled) return;
			bReloadScheduled = true;
			CurrentState = EBotState::Finished;
			UE_LOG(LogTemp, Warning, TEXT("봇: 골 도달 → 데이터 정산 후 레벨 리로드"));
			if (GoalActor)
				GoalActor->ProcessGoalForActor(GetPawn());
			FTimerHandle ReloadTimer;
			TWeakObjectPtr<ABotPlayController> WeakThis1(this);
			GetWorldTimerManager().SetTimer(ReloadTimer, [WeakThis1]()
			{
				if (WeakThis1.IsValid())
				{
					FString LevelName = WeakThis1->GetWorld()->GetName();
					UGameplayStatics::OpenLevel(WeakThis1.Get(), FName(*LevelName));
				}
			}, 1.5f, false);
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
// 몬스터 전투 (근접 or 원거리)
// =============================================
void ABotPlayController::CheckAndEngageMonsters()
{
	if (CurrentState != EBotState::Roaming) return;
	if (bGoalIsNextTarget) return;

	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	// 600u 이내 거리 기반 탐색 (overlap 아님)
	ACharacter* Monster = FindNearestMonster(MyPawn, 600.f);
	if (!Monster || Monster == LastHandledMonster) return;

	LastHandledMonster = Monster;
	float Dist = FVector::Dist(MyPawn->GetActorLocation(), Monster->GetActorLocation());

	// CombatWeight 낮으면 도주
	if (FMath::FRand() > CombatWeight)
	{
		FVector FleeDir = (MyPawn->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal();
		FVector FleeTarget = MyPawn->GetActorLocation() + FleeDir * 1000.f;

		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys)
		{
			FNavLocation NavLoc;
			if (NavSys->ProjectPointToNavigation(FleeTarget, NavLoc, FVector(500.f, 500.f, 200.f)))
				FleeTarget = NavLoc.Location;
		}
		CurrentState = EBotState::AvoidingMonster;
		MoveToLocation(FleeTarget, 100.f);
		UE_LOG(LogTemp, Warning, TEXT("봇: 몬스터 회피 (거리 %.0fu)"), Dist);
		return;
	}

	// MeleePreference → 근접 or 원거리 결정
	if (FMath::FRand() < MeleePreference)
	{
		// 근접 공격
		bIsRangedAttack = false;
		if (Dist <= 200.f)
		{
			KillMonster(MyPawn, Monster, true);
			LastHandledMonster = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("봇: 즉시 근접 처치!"));
		}
		else
		{
			CurrentState = EBotState::ApproachingMonster;
			MoveToActor(Monster, 10.f);
			UE_LOG(LogTemp, Warning, TEXT("봇: 근접 접근 중 (%.0fu)"), Dist);
		}
	}
	else
	{
		// 원거리 공격 — 400u 이상이면 현 위치에서, 그 이하이면 뒤로 물러난 후 처치
		bIsRangedAttack = true;
		if (Dist >= 400.f)
		{
			// 충분히 멀면 즉시 원거리 처치
			KillMonster(MyPawn, Monster, false);
			LastHandledMonster = nullptr;
			UE_LOG(LogTemp, Warning, TEXT("봇: 원거리 즉시 처치! (%.0fu)"), Dist);
		}
		else
		{
			// 가까우면 뒤로 물러난 후 OnMoveCompleted에서 처치
			FVector BackDir = (MyPawn->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal();
			FVector BackTarget = MyPawn->GetActorLocation() + BackDir * (500.f - Dist);

			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
			if (NavSys)
			{
				FNavLocation NavLoc;
				if (NavSys->ProjectPointToNavigation(BackTarget, NavLoc, FVector(500.f, 500.f, 200.f)))
					BackTarget = NavLoc.Location;
			}
			CurrentState = EBotState::ApproachingMonster;
			MoveToLocation(BackTarget, 50.f);
			UE_LOG(LogTemp, Warning, TEXT("봇: 원거리용 후퇴 중 (%.0fu→500u)"), Dist);
		}
	}
}

// =============================================
// 장애물 판단 (거리 기반 선제 감지)
// =============================================
void ABotPlayController::CheckAndHandleObstacles()
{
	if (CurrentState != EBotState::Roaming) return;
	if (bGoalIsNextTarget) return;

	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	// 600u 이내 가장 가까운 장애물 탐색 (overlap 아님 → 벽에 박히기 전에 감지)
	AAnalysisObstacle* Obstacle = FindNearestObstacle(MyPawn, 600.f);

	if (!Obstacle)
	{
		LastHandledObstacle = nullptr;
		return;
	}

	if (Obstacle == LastHandledObstacle) return;

	LastHandledObstacle = Obstacle;

	FVector MyLoc = MyPawn->GetActorLocation();
	FVector ObstacleLoc = Obstacle->GetActorLocation();
	FVector ToObstacle = (ObstacleLoc - MyLoc).GetSafeNormal();

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

	UPlayerTrackerComponent* Tracker = MyPawn->FindComponentByClass<UPlayerTrackerComponent>();

	if (FMath::FRand() < ObstaclePassPreference)
	{
		// 돌파: 장애물 너머로 실제 이동 → InnerZone overlap이 PassedObstacleCount++ 처리
		FVector PassTarget = ObstacleLoc + ToObstacle * 500.f;

		if (NavSys)
		{
			FNavLocation NavLoc;
			if (NavSys->ProjectPointToNavigation(PassTarget, NavLoc, FVector(500.f, 500.f, 200.f)))
				PassTarget = NavLoc.Location;
		}

		CurrentState = EBotState::PassingObstacle;
		MoveToLocation(PassTarget, 100.f);
		UE_LOG(LogTemp, Log, TEXT("봇: 장애물 실제 돌파 이동 시작"));
	}
	else
	{
		// 회피: 측면으로 이동 → 수동 카운트 (OuterZone 진입 보장이 어려워 직접 집계)
		if (Tracker) Tracker->AvoidedObstacleCount++;

		FVector SideDir = FVector(-ToObstacle.Y, ToObstacle.X, 0.f);
		if (FMath::RandBool()) SideDir = -SideDir;
		FVector AvoidTarget = MyLoc + SideDir * 700.f;

		if (NavSys)
		{
			FNavLocation NavLoc;
			if (NavSys->ProjectPointToNavigation(AvoidTarget, NavLoc, FVector(500.f, 500.f, 200.f)))
				AvoidTarget = NavLoc.Location;
		}

		CurrentState = EBotState::AvoidingObstacle;
		MoveToLocation(AvoidTarget, 100.f);
		UE_LOG(LogTemp, Log, TEXT("봇: 장애물 측면 회피"));
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
		if (GoalActor) GoalActor->ProcessGoalForActor(GetPawn());
		return;
	}

	bGoalIsNextTarget = true;
	FVector GoalLoc = GoalActor->GetActorLocation();

	// GoalTrigger 위치를 NavMesh로 투영 (NavMesh 밖에 있으면 경로 탐색 실패)
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		// 반경을 크게 잡아 NavMesh 위 가장 가까운 점을 찾음
		if (NavSys->ProjectPointToNavigation(GoalLoc, NavLoc, FVector(1000.f, 1000.f, 500.f)))
			GoalLoc = NavLoc.Location;
	}

	UE_LOG(LogTemp, Warning, TEXT("봇: 웨이포인트 %d개 완료 → 골로 이동 (마지막 웨이포인트)"), WaypointsVisited);

	EPathFollowingRequestResult::Type Result = MoveToLocation(GoalLoc, 50.f);

	TWeakObjectPtr<ABotPlayController> WeakThis2(this);
	auto FinishRun = [this, WeakThis2]()
	{
		if (bReloadScheduled) return;
		bReloadScheduled = true;
		CurrentState = EBotState::Finished;
		if (GoalActor) GoalActor->ProcessGoalForActor(GetPawn());
		FTimerHandle ReloadTimer;
		GetWorldTimerManager().SetTimer(ReloadTimer, [WeakThis2]()
		{
			if (WeakThis2.IsValid())
			{
				FString LevelName = WeakThis2->GetWorld()->GetName();
				UGameplayStatics::OpenLevel(WeakThis2.Get(), FName(*LevelName));
			}
		}, 1.5f, false);
	};

	if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogTemp, Warning, TEXT("봇: 골 도달(이미 위치) → 정산"));
		FinishRun();
	}
	else if (Result == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogTemp, Warning, TEXT("봇: 골 경로 실패 → MoveToActor 재시도"));
		EPathFollowingRequestResult::Type FallbackResult = MoveToActor(GoalActor, 100.f);
		// RequestSuccessful 이 아니면 (AlreadyAtGoal 포함) 즉시 정산
		if (FallbackResult != EPathFollowingRequestResult::RequestSuccessful)
		{
			UE_LOG(LogTemp, Warning, TEXT("봇: 골 즉시 정산 (FallbackResult=%d)"), (int32)FallbackResult);
			FinishRun();
		}
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