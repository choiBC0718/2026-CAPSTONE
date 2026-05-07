#include "StageGoalTrigger.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "QuadtreeManager.h"
#include "AI/PlayerBehaviorLearner.h"
#include "AI/PlayerTrackerComponent.h"

AStageGoalTrigger::AStageGoalTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    GoalZone = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalZone"));
    RootComponent = GoalZone;
    GoalZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));
    GoalZone->SetCollisionProfileName(TEXT("Trigger"));
    GoalZone->OnComponentBeginOverlap.AddDynamic(this, &AStageGoalTrigger::OnOverlapBegin);
}

void AStageGoalTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && (OtherActor != this))
    {
        UPlayerTrackerComponent* Tracker = OtherActor->FindComponentByClass<UPlayerTrackerComponent>();
        
        if (!Tracker) return;

        GoalZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        UE_LOG(LogTemp, Warning, TEXT("5차원 정규화 데이터 정산을 시작합니다."));

        float PlayTime = GetWorld()->GetTimeSeconds();

        AQuadtreeManager* Quadtree = Cast<AQuadtreeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AQuadtreeManager::StaticClass()));
        APlayerBehaviorLearner* Learner = Cast<APlayerBehaviorLearner>(UGameplayStatics::GetActorOfClass(GetWorld(), APlayerBehaviorLearner::StaticClass()));

        if (Quadtree && Learner)
        {
            // 1. 노드 방문율 (0~1) [핵심 변경 사항]
            TArray<FQuadtreeNode*> AllNodes = Quadtree->GetAllLeafNodes();
            int32 TotalLeafNodes = AllNodes.Num();
            int32 VisitedCount = 0;
            
            for (FQuadtreeNode* Node : AllNodes)
            {
                if (Node->VisitCount > 0) VisitedCount++;
            }

            float NormalizedVisit = 0.0f;
            if (TotalLeafNodes > 0)
            {
                // 유저가 도달해야 할 목표 타일 수 (예: 100개 중 40개)
                float TargetVisitCount = static_cast<float>(TotalLeafNodes) * ExpectedMaxVisitRatio;
                NormalizedVisit = static_cast<float>(VisitedCount) / TargetVisitCount;
                
                // 타일을 40% 이상 초과해서 밟았더라도 1.0을 넘지 않도록 제한
                NormalizedVisit = FMath::Clamp(NormalizedVisit, 0.0f, 1.0f); 
            }

            // 2. 플레이 시간 (0~1) [기준시간 120초로 변경됨]
            float NormalizedTime = FMath::Clamp(PlayTime / MaxExpectedPlayTime, 0.0f, 1.0f);

            // 3. 장애물 돌파율 (0~1)
            int32 TotalEncountered = Tracker->PassedObstacleCount + Tracker->AvoidedObstacleCount;
            float NormalizedPass = (TotalEncountered > 0)
                ? static_cast<float>(Tracker->PassedObstacleCount) / static_cast<float>(TotalEncountered)
                : 0.0f;

            // 4. 몬스터 처치율 (0~1)
            float NormalizedKill = (Tracker->TotalSpawnedMonsters > 0)
                ? static_cast<float>(Tracker->KilledMonsterCount) / static_cast<float>(Tracker->TotalSpawnedMonsters)
                : 0.0f;

            // 5. 근접 공격 선호도 (0~1)
            float NormalizedMelee = (Tracker->KilledMonsterCount > 0)
                ? static_cast<float>(Tracker->MeleeKillCount) / static_cast<float>(Tracker->KilledMonsterCount)
                : 0.0f;

            UE_LOG(LogTemp, Log, TEXT("정규화 결과 -> 방문:%.3f / 시간:%.3f / 돌파:%.3f / 처치:%.3f / 근접:%.3f"), 
                   NormalizedVisit, NormalizedTime, NormalizedPass, NormalizedKill, NormalizedMelee);

            FPlayerBehaviorData FinalData;
            FinalData.VisitedNodeCount = NormalizedVisit;
            FinalData.PlayTime = NormalizedTime;
            FinalData.PassRatio = NormalizedPass;
            FinalData.KillRatio = NormalizedKill;
            FinalData.MeleeRatio = NormalizedMelee;

            Learner->ProcessPlayerData(FinalData);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("맵에 QuadtreeManager OR PlayerBehaviorLearner X"));
        }
    }
}