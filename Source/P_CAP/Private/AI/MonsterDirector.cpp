#include "MonsterDirector.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AI/PlayerTrackerComponent.h" 
#include "BaseMonster.h"
#include "EngineUtils.h"               

AMonsterDirector::AMonsterDirector()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    RootComponent = SpawnVolume;
    SpawnVolume->SetBoxExtent(FVector(1000.f, 1000.f, 100.f));
}

void AMonsterDirector::BeginPlay()
{
    Super::BeginPlay();

    // 플레이어 캐릭터 완전 초기화 대기 후 스폰
    FTimerHandle SpawnTimer;
    GetWorld()->GetTimerManager().SetTimer(SpawnTimer, [this]()
    {
        // [변경됨] 기본 성향 구조체 준비 (기본값 0.5)
        FPlayerTendencyModifier CurrentTendency;

        // 맵에 존재하는 APlayerBehaviorLearner를 찾아서 최신 유저 성향을 가져옵니다.
        APlayerBehaviorLearner* Learner = Cast<APlayerBehaviorLearner>(
            UGameplayStatics::GetActorOfClass(GetWorld(), APlayerBehaviorLearner::StaticClass()));
        
        if (Learner)
        {
            CurrentTendency = Learner->GetCurrentPlayerTendency();
        }

        // [변경됨] 이름표 대신 4차원 성향 데이터를 통째로 넘겨줍니다.
        SpawnMonstersByTendency(CurrentTendency);
    }, 1.0f, false);
}

// [변경됨] 함수 이름과 매개변수가 FPlayerTendencyModifier로 변경되었습니다.
void AMonsterDirector::SpawnMonstersByTendency(const FPlayerTendencyModifier& PlayerTendency)
{
    if (!MonsterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterDirector에 MonsterClass가 할당되지 않았습니다."));
        return;
    }

    FVector Origin = SpawnVolume->Bounds.Origin;
    FVector BoxExtent = SpawnVolume->Bounds.BoxExtent;

    int32 ExplorerSpawnCount = 0;
    int32 SpeedRunnerSpawnCount = 0;

    for (int32 i = 0; i < MonsterCount; i++)
    {
        FVector SpawnLocation = Origin;

        // [핵심 변경됨] ExplorationRate(0.0~1.0)를 확률로 사용하여 스폰 위치를 결정합니다.
        // 예: 수치가 0.7이면 70% 확률로 외곽(탐험형 패턴)에, 30% 확률로 중앙(직진형 패턴)에 배치됩니다.
        if (FMath::FRand() < PlayerTendency.ExplorationRate)
        {
            SpawnLocation = GetExplorerSpawnPoint(Origin, BoxExtent);
            ExplorerSpawnCount++;
        }
        else
        {
            SpawnLocation = GetSpeedRunnerSpawnPoint(Origin, BoxExtent);
            SpeedRunnerSpawnCount++;
        }

        ABaseMonster* SpawnedEntity = GetWorld()->SpawnActor<ABaseMonster>(MonsterClass, SpawnLocation, FRotator::ZeroRotator);
        
        if (SpawnedEntity)
        {
            // GetPlayerCharacter는 봇 모드에서 null 반환 → 폰 직접 탐색
            for (TActorIterator<APawn> It(GetWorld()); It; ++It)
            {
                UPlayerTrackerComponent* Tracker = It->FindComponentByClass<UPlayerTrackerComponent>();
                if (Tracker)
                {
                    Tracker->TotalSpawnedMonsters++;
                    break;
                }
            }
        }
    }
    
    // [변경됨] 스폰 결과 로그도 디테일하게 변경
    UE_LOG(LogTemp, Warning, TEXT("=== 스폰 완료! 총 %d마리 ==="), MonsterCount);
    UE_LOG(LogTemp, Warning, TEXT("  ㄴ 적용된 유저 탐색률: %.2f (%.0f%% 확률로 외곽 배치)"), 
           PlayerTendency.ExplorationRate, PlayerTendency.ExplorationRate * 100.f);
    UE_LOG(LogTemp, Warning, TEXT("  ㄴ 실제 배치 결과 - 외곽 분산: %d마리 / 중앙 집중: %d마리"), 
           ExplorerSpawnCount, SpeedRunnerSpawnCount);
}

FVector AMonsterDirector::GetSpeedRunnerSpawnPoint(FVector Center, FVector Extent)
{
    float RadiusX = Extent.X * SpeedRunnerCenterRatio;
    float RadiusY = Extent.Y * SpeedRunnerCenterRatio;

    float RandX = FMath::RandRange(-RadiusX, RadiusX);
    float RandY = FMath::RandRange(-RadiusY, RadiusY);

    return Center + FVector(RandX, RandY, 0.f);
}

FVector AMonsterDirector::GetExplorerSpawnPoint(FVector Center, FVector Extent)
{
    bool bSpawnOnXEdge = FMath::RandBool();
    
    float RandX = 0.f;
    float RandY = 0.f;

    if (bSpawnOnXEdge)
    {
        RandX = FMath::RandRange(Extent.X * ExplorerEdgeStartRatio, Extent.X) * (FMath::RandBool() ? 1 : -1);
        RandY = FMath::RandRange(-Extent.Y, Extent.Y);
    }
    else
    {
        RandX = FMath::RandRange(-Extent.X, Extent.X);
        RandY = FMath::RandRange(Extent.Y * ExplorerEdgeStartRatio, Extent.Y) * (FMath::RandBool() ? 1 : -1);
    }

    return Center + FVector(RandX, RandY, 0.f);
}