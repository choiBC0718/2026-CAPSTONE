#include "MonsterDirector.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AI/PlayerTrackerComponent.h"
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

void AMonsterDirector::SpawnMonstersByTendency(const FPlayerTendencyModifier& PlayerTendency)
{
    if (!MonsterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("MonsterDirector에 MonsterClass가 할당되지 않았습니다."));
        return;
    }

    SpawnedMonsters.Empty();

    int32 ExplorerCount = 0, SpeedRunnerCount = 0;
    for (int32 i = 0; i < MonsterCount; i++)
    {
        ACharacter* Spawned = SpawnOneMonster(PlayerTendency);
        if (Spawned)
        {
            SpawnedMonsters.Add(Spawned);
            if (FMath::FRand() < PlayerTendency.ExplorationRate) ExplorerCount++;
            else SpeedRunnerCount++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("=== 스폰 완료! %d마리 (외곽:%d / 중앙:%d) ==="),
           SpawnedMonsters.Num(), ExplorerCount, SpeedRunnerCount);

    // TotalSpawnedMonsters를 실제 배치 수가 아닌 MonsterCount 기준으로 고정
    // (배치 실패로 인한 분모 불일치 방지 → KillRatio 정확도 확보)
    for (TActorIterator<APawn> It(GetWorld()); It; ++It)
    {
        UPlayerTrackerComponent* Tracker = It->FindComponentByClass<UPlayerTrackerComponent>();
        if (Tracker) { Tracker->TotalSpawnedMonsters = MonsterCount; break; }
    }

    if (RespawnInterval > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this,
            &AMonsterDirector::CheckAndRespawn, RespawnInterval, true);
    }
}

ACharacter* AMonsterDirector::SpawnOneMonster(const FPlayerTendencyModifier& Tendency)
{
    FVector Origin    = SpawnVolume->Bounds.Origin;
    FVector BoxExtent = SpawnVolume->Bounds.BoxExtent;

    // 최소 간격(600u) 만족하는 위치를 최대 10회 시도
    const float MinDist = 600.f;
    FVector SpawnLocation;
    bool bFound = false;
    for (int32 Try = 0; Try < 10; Try++)
    {
        FVector Candidate = (FMath::FRand() < Tendency.ExplorationRate)
            ? GetExplorerSpawnPoint(Origin, BoxExtent)
            : GetSpeedRunnerSpawnPoint(Origin, BoxExtent);

        bool bTooClose = false;
        for (auto& Weak : SpawnedMonsters)
        {
            if (Weak.IsValid() && FVector::Dist(Candidate, Weak->GetActorLocation()) < MinDist)
            {
                bTooClose = true;
                break;
            }
        }
        if (!bTooClose) { SpawnLocation = Candidate; bFound = true; break; }
    }
    if (!bFound) return nullptr; // 공간 없으면 스킵

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ACharacter* Spawned = GetWorld()->SpawnActor<ACharacter>(
        MonsterClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

    return Spawned;
}

void AMonsterDirector::CheckAndRespawn()
{
    if (!MonsterClass) return;

    // 살아있는 몬스터 수 계산
    int32 AliveCount = 0;
    for (auto& Weak : SpawnedMonsters)
        if (Weak.IsValid()) AliveCount++;

    int32 ToSpawn = MonsterCount - AliveCount;
    if (ToSpawn <= 0) return;

    FPlayerTendencyModifier DefaultTendency;
    APlayerBehaviorLearner* Learner = Cast<APlayerBehaviorLearner>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APlayerBehaviorLearner::StaticClass()));
    if (Learner) DefaultTendency = Learner->GetCurrentPlayerTendency();

    for (int32 i = 0; i < ToSpawn; i++)
    {
        ACharacter* Spawned = SpawnOneMonster(DefaultTendency);
        if (Spawned) SpawnedMonsters.Add(Spawned);
    }

    UE_LOG(LogTemp, Log, TEXT("MonsterDirector: %d마리 보충 스폰 (현재 %d/%d)"),
           ToSpawn, AliveCount + ToSpawn, MonsterCount);
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