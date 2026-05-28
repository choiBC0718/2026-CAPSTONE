#include "StageGoalTrigger.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "QuadtreeManager.h"
#include "AI/PlayerBehaviorLearner.h"
#include "AI/PlayerTrackerComponent.h"
#include "DrawDebugHelpers.h"

AStageGoalTrigger::AStageGoalTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    GoalZone = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalZone"));
    RootComponent = GoalZone;
    GoalZone->SetBoxExtent(FVector(300.f, 300.f, 300.f));
    GoalZone->SetCollisionProfileName(TEXT("Trigger"));
}

void AStageGoalTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Blueprint 덮어쓰기 방지 — 플레이어가 막히지 않도록 순수 Overlap Trigger로 강제
    GoalZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GoalZone->SetCollisionResponseToAllChannels(ECR_Overlap);
    GoalZone->SetGenerateOverlapEvents(true);

    GoalZone->OnComponentBeginOverlap.RemoveDynamic(this, &AStageGoalTrigger::OnOverlapBegin);
    GoalZone->OnComponentBeginOverlap.AddDynamic(this, &AStageGoalTrigger::OnOverlapBegin);

    FVector Loc = GoalZone->GetComponentLocation();
    FVector Ext = GoalZone->GetScaledBoxExtent();
    UE_LOG(LogTemp, Warning, TEXT("[GoalTrigger] 위치=(%.0f, %.0f, %.0f)  크기=(%.0f, %.0f, %.0f)"),
           Loc.X, Loc.Y, Loc.Z, Ext.X, Ext.Y, Ext.Z);
    DrawDebugBox(GetWorld(), Loc, Ext, FColor::Green, false, 30.f, 0, 10.f);
}

void AStageGoalTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ProcessGoalForActor(OtherActor);
}

void AStageGoalTrigger::ProcessGoalForActor(AActor* OtherActor)
{
    if (!OtherActor || OtherActor == this) return;

    UE_LOG(LogTemp, Warning, TEXT("[GoalTrigger] Overlap: %s"), *OtherActor->GetName());

    UPlayerTrackerComponent* Tracker = OtherActor->FindComponentByClass<UPlayerTrackerComponent>();
    if (!Tracker)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GoalTrigger] PlayerTrackerComponent 없음 — 스킵"));
        return;
    }

    // 중복 처리 방지 (물리 overlap + 봇 직접 호출 동시에 올 경우)
    GoalZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UE_LOG(LogTemp, Warning, TEXT("5차원 정규화 데이터 정산을 시작합니다."));

    float TimeDilation = GetWorld()->GetWorldSettings()->TimeDilation;
    float PlayTime = GetWorld()->GetTimeSeconds() / FMath::Max(TimeDilation, 0.01f);

    AQuadtreeManager* Quadtree = Cast<AQuadtreeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AQuadtreeManager::StaticClass()));
    APlayerBehaviorLearner* Learner = Cast<APlayerBehaviorLearner>(UGameplayStatics::GetActorOfClass(GetWorld(), APlayerBehaviorLearner::StaticClass()));

    if (Quadtree && Learner)
    {
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
            float TargetVisitCount = static_cast<float>(TotalLeafNodes) * ExpectedMaxVisitRatio;
            NormalizedVisit = FMath::Clamp(static_cast<float>(VisitedCount) / TargetVisitCount, 0.0f, 1.0f);
        }

        float NormalizedTime = FMath::Clamp(PlayTime / MaxExpectedPlayTime, 0.0f, 1.0f);

        int32 TotalEncountered = Tracker->PassedObstacleCount + Tracker->AvoidedObstacleCount;
        float NormalizedPass = (TotalEncountered > 0)
            ? static_cast<float>(Tracker->PassedObstacleCount) / static_cast<float>(TotalEncountered)
            : 0.0f;

        float NormalizedKill = (Tracker->TotalSpawnedMonsters > 0)
            ? static_cast<float>(Tracker->KilledMonsterCount) / static_cast<float>(Tracker->TotalSpawnedMonsters)
            : 0.0f;

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

        // 플레이어 성향 분석 결과 상세 로그
        FPlayerTendencyModifier T = Learner->GetCurrentPlayerTendency();

        auto TendencyLabel = [](float V, const TCHAR* Low, const TCHAR* Mid, const TCHAR* High) -> const TCHAR*
        {
            return V >= 0.66f ? High : V >= 0.33f ? Mid : Low;
        };

        UE_LOG(LogTemp, Warning, TEXT("========== 플레이어 성향 분석 결과 =========="));
        UE_LOG(LogTemp, Warning, TEXT("  [원시 데이터] 방문:%.2f | 시간:%.2f | 돌파:%.2f | 처치:%.2f | 근접:%.2f"),
               FinalData.VisitedNodeCount, FinalData.PlayTime,
               FinalData.PassRatio, FinalData.KillRatio, FinalData.MeleeRatio);
        UE_LOG(LogTemp, Warning, TEXT("  탐험율    (ExplorationRate)  : %.2f  → %s"),
               T.ExplorationRate,
               TendencyLabel(T.ExplorationRate, TEXT("직진형 (목표만 향해 달림)"), TEXT("보통"), TEXT("탐험형 (구석구석 탐색)")));
        UE_LOG(LogTemp, Warning, TEXT("  전투적극성 (CombatAggression): %.2f  → %s"),
               T.CombatAggression,
               TendencyLabel(T.CombatAggression, TEXT("회피형 (몬스터 피해다님)"), TEXT("보통"), TEXT("전투형 (적극 교전)")));
        UE_LOG(LogTemp, Warning, TEXT("  근접선호   (MeleePreference) : %.2f  → %s"),
               T.MeleePreference,
               TendencyLabel(T.MeleePreference, TEXT("원거리 선호"), TEXT("균형"), TEXT("근접 선호")));
        UE_LOG(LogTemp, Warning, TEXT("  장애물돌파 (ObstacleBypass)  : %.2f  → %s"),
               T.ObstacleBypass,
               TendencyLabel(T.ObstacleBypass, TEXT("우회형 (장애물 피해감)"), TEXT("보통"), TEXT("돌파형 (장애물 뚫고 지나감)")));

        // 가장 높은 수치 2개를 기반으로 종합 성향 한 줄 요약
        TArray<TPair<float, FString>> Items = {
            { T.ExplorationRate,  T.ExplorationRate  >= 0.5f ? TEXT("탐험형") : TEXT("직진형") },
            { T.CombatAggression, T.CombatAggression >= 0.5f ? TEXT("전투형") : TEXT("회피형") },
            { T.MeleePreference,  T.MeleePreference  >= 0.5f ? TEXT("근접형") : TEXT("원거리형") },
            { T.ObstacleBypass,   T.ObstacleBypass   >= 0.5f ? TEXT("돌파형") : TEXT("우회형") }
        };
        Items.Sort([](const TPair<float,FString>& A, const TPair<float,FString>& B){ return A.Key > B.Key; });
        FString Summary = FString::Printf(TEXT("  [종합 성향] %s / %s  (탐험%.0f%% 전투%.0f%% 근접%.0f%% 돌파%.0f%%)"),
               *Items[0].Value, *Items[1].Value,
               T.ExplorationRate*100.f, T.CombatAggression*100.f,
               T.MeleePreference*100.f, T.ObstacleBypass*100.f);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Summary);
        UE_LOG(LogTemp, Warning, TEXT("============================================="));

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, Summary);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("맵에 QuadtreeManager OR PlayerBehaviorLearner X"));
    }
}