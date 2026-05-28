#include "AI/PlayerBehaviorLearner.h"
#include "Kismet/GameplayStatics.h"
#include "AI/LearnerSaveGame.h"       
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

APlayerBehaviorLearner::APlayerBehaviorLearner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APlayerBehaviorLearner::BeginPlay()
{
    Super::BeginPlay();
    LoadLearnerData();

    if (GEngine)
    {
        FString Msg = FString::Printf(TEXT("AI 뇌 로드 완료 (K-Means / K=%d / 히스토리:%d건)"), K, PlayHistory.Num());
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, Msg);
    }
}

void APlayerBehaviorLearner::ProcessPlayerData(FPlayerBehaviorData PlayerDataPoint)
{
    if (PlayerDataPoint.PlayTime < MinValidPlayTimeRatio)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Learner] 비정상 런 무시 (PlayTime=%.3f, 최소=%.3f)"),
               PlayerDataPoint.PlayTime, MinValidPlayTimeRatio);
        return;
    }

    PlayHistory.Add(PlayerDataPoint);

    while (PlayHistory.Num() > MaxHistorySize)
    {
        PlayHistory.RemoveAt(0);
    }

    UE_LOG(LogTemp, Warning, TEXT("플레이 데이터 수집 완료 (총 %d건)"), PlayHistory.Num());

    if (PlayHistory.Num() >= MinDataForClustering)
    {
        RunKMeans();

        // [변경됨] 이름표(페르소나) 대신 소속된 군집 번호만 로그로 띄움
        int32 ClusterIdx = FindNearestCentroid(PlayerDataPoint);
        FString Msg = FString::Printf(TEXT("K-Means 분류 완료: 현재 플레이어는 Cluster[%d] 성향입니다."), ClusterIdx);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, Msg);
        }

        ExportAllDataToCSV();
    }
    else
    {
        FString Msg = FString::Printf(TEXT("데이터 수집 중... (%d / %d)"), PlayHistory.Num(), MinDataForClustering);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, Msg);
        }
    }

    SaveLearnerData();
}

void APlayerBehaviorLearner::RunKMeans()
{
    UE_LOG(LogTemp, Warning, TEXT("=== K-Means 클러스터링 시작 (K=%d, 데이터=%d건) ==="), K, PlayHistory.Num());

    InitializeCentroidsKMeansPP();

    for (int32 Iter = 0; Iter < MaxIterations; Iter++)
    {
        TArray<TArray<int32>> ClusterMembers;
        ClusterMembers.SetNum(K);

        for (int32 i = 0; i < PlayHistory.Num(); i++)
        {
            int32 NearestIdx = FindNearestCentroid(PlayHistory[i]);
            ClusterMembers[NearestIdx].Add(i);
        }

        TArray<FPlayerBehaviorData> NewCentroids;
        NewCentroids.SetNum(K);
        bool bConverged = true;

        for (int32 c = 0; c < K; c++)
        {
            if (ClusterMembers[c].Num() == 0)
            {
                NewCentroids[c] = Centroids[c];
                continue;
            }

            FPlayerBehaviorData Sum;
            for (int32 DataIdx : ClusterMembers[c])
            {
                Sum = Sum + PlayHistory[DataIdx];
            }
            NewCentroids[c] = Sum / static_cast<float>(ClusterMembers[c].Num());

            float Moved = FPlayerBehaviorData::Distance(Centroids[c], NewCentroids[c]);
            if (Moved > ConvergenceThreshold)
            {
                bConverged = false;
            }
        }

        Centroids = NewCentroids;

        if (bConverged)
        {
            UE_LOG(LogTemp, Warning, TEXT("K-Means 수렴 완료! (%d회 반복)"), Iter + 1);
            break;
        }
    }

    bIsClusteringReady = true;

    // 최종 군집 크기 기록
    ClusterMemberCounts.SetNum(K);
    for (int32& Count : ClusterMemberCounts) Count = 0;
    for (int32 i = 0; i < PlayHistory.Num(); i++)
    {
        ClusterMemberCounts[FindNearestCentroid(PlayHistory[i])]++;
    }

    for (int32 c = 0; c < K; c++)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Cluster[%d] = (V:%.3f T:%.3f P:%.3f K:%.3f M:%.3f) 멤버:%d"),
               c,
               Centroids[c].VisitedNodeCount, Centroids[c].PlayTime,
               Centroids[c].PassRatio, Centroids[c].KillRatio, Centroids[c].MeleeRatio,
               ClusterMemberCounts[c]);
    }
}

void APlayerBehaviorLearner::InitializeCentroidsKMeansPP()
{
    Centroids.Empty();

    int32 FirstIdx = FMath::RandRange(0, PlayHistory.Num() - 1);
    Centroids.Add(PlayHistory[FirstIdx]);

    for (int32 c = 1; c < K; c++)
    {
        float BestDist = -1.f;
        int32 BestIdx = 0;

        for (int32 i = 0; i < PlayHistory.Num(); i++)
        {
            float MinDistToExisting = TNumericLimits<float>::Max();
            for (int32 e = 0; e < Centroids.Num(); e++)
            {
                float Dist = FPlayerBehaviorData::Distance(PlayHistory[i], Centroids[e]);
                if (Dist < MinDistToExisting)
                {
                    MinDistToExisting = Dist;
                }
            }

            if (MinDistToExisting > BestDist)
            {
                BestDist = MinDistToExisting;
                BestIdx = i;
            }
        }

        Centroids.Add(PlayHistory[BestIdx]);
    }

    UE_LOG(LogTemp, Log, TEXT("K-Means++ 초기 센트로이드 %d개 선택 완료"), K);
}

int32 APlayerBehaviorLearner::FindNearestCentroid(const FPlayerBehaviorData& DataPoint) const
{
    int32 NearestIdx = 0;
    float MinDist = TNumericLimits<float>::Max();

    for (int32 c = 0; c < Centroids.Num(); c++)
    {
        float Dist = FPlayerBehaviorData::Distance(DataPoint, Centroids[c]);
        if (Dist < MinDist)
        {
            MinDist = Dist;
            NearestIdx = c;
        }
    }

    return NearestIdx;
}

// =============================================
// [변경됨] 맵 팀이 호출할 데이터 추출 API
// =============================================
FPlayerTendencyModifier APlayerBehaviorLearner::GetCurrentPlayerTendency()
{
    FPlayerTendencyModifier Tendency; // 기본 생성자에서 모두 0.5로 세팅됨

    // 예외 처리: 데이터가 없거나 학습 전이면 기본값 반환
    if (!bIsClusteringReady || Centroids.Num() == 0 || PlayHistory.Num() == 0)
    {
        return Tendency;
    }

    FPlayerBehaviorData LatestPlay = PlayHistory.Last();
    int32 ClusterIdx = FindNearestCentroid(LatestPlay);

    // 소속 클러스터 멤버가 너무 적으면 가장 큰 클러스터로 대체
    if (ClusterMemberCounts.IsValidIndex(ClusterIdx) && ClusterMemberCounts[ClusterIdx] < MinClusterSize)
    {
        int32 DominantCluster = 0;
        for (int32 c = 1; c < K; c++)
            if (ClusterMemberCounts.IsValidIndex(c) && ClusterMemberCounts[c] > ClusterMemberCounts[DominantCluster])
                DominantCluster = c;
        UE_LOG(LogTemp, Warning, TEXT("[Tendency] Cluster[%d] 크기 부족(%d<%d) → Cluster[%d]로 대체"),
               ClusterIdx, ClusterMemberCounts[ClusterIdx], MinClusterSize, DominantCluster);
        ClusterIdx = DominantCluster;
    }

    const FPlayerBehaviorData& C = Centroids[ClusterIdx];

    Tendency.ExplorationRate = C.VisitedNodeCount;
    Tendency.CombatAggression = C.KillRatio;
    Tendency.MeleePreference = C.MeleeRatio;
    Tendency.ObstacleBypass = C.PassRatio;

    return Tendency;
}

void APlayerBehaviorLearner::SaveLearnerData()
{
    USaveGame* SaveGameObj = UGameplayStatics::CreateSaveGameObject(ULearnerSaveGame::StaticClass());
    ULearnerSaveGame* SaveInstance = Cast<ULearnerSaveGame>(SaveGameObj);
    
    if (SaveInstance)
    {
        SaveInstance->SavedPlayHistory = PlayHistory;
        SaveInstance->SavedCentroids = Centroids;
        SaveInstance->bSavedClusteringReady = bIsClusteringReady;
		// [변경됨] Persona 저장 로직 삭제

        bool bIsSaved = UGameplayStatics::SaveGameToSlot(SaveInstance, TEXT("AIBrainData"), 0);

        if (bIsSaved)
        {
            UE_LOG(LogTemp, Warning, TEXT("K-Means 뇌 저장 완료 (히스토리:%d / 클러스터:%d)"), 
                   PlayHistory.Num(), Centroids.Num());
        }
    }
}

void APlayerBehaviorLearner::LoadLearnerData()
{
    if (!UGameplayStatics::DoesSaveGameExist(TEXT("AIBrainData"), 0))
    {
        UE_LOG(LogTemp, Warning, TEXT("세이브 파일 없음. 빈 상태로 시작합니다."));
        return;
    }

    USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(TEXT("AIBrainData"), 0);
    ULearnerSaveGame* LoadInstance = Cast<ULearnerSaveGame>(LoadedGame);

    if (LoadInstance)
    {
        PlayHistory = LoadInstance->SavedPlayHistory;
        Centroids = LoadInstance->SavedCentroids;
        bIsClusteringReady = LoadInstance->bSavedClusteringReady;
		// [변경됨] Persona 로드 로직 삭제

        UE_LOG(LogTemp, Warning, TEXT("세이브 로드 완료 (히스토리:%d / 클러스터링:%s)"),
               PlayHistory.Num(),
               bIsClusteringReady ? TEXT("준비됨") : TEXT("수집중"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("세이브 파일 변환 실패"));
    }
}

void APlayerBehaviorLearner::ExportDataToCSV(const FPlayerBehaviorData& PlayerData, const FString& ClusterLabel)
{
    FString FilePath = FPaths::ProjectSavedDir() + TEXT("Logs/AILearningData.csv");

    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
    {
        FString Header = TEXT("VisitedNodeCount,PlayTime,PassRatio,KillRatio,MeleeRatio,Cluster\n");
        FFileHelper::SaveStringToFile(Header, *FilePath);
    }

    FString DataLine = FString::Printf(TEXT("%f,%f,%f,%f,%f,%s\n"),
                                       PlayerData.VisitedNodeCount,
                                       PlayerData.PlayTime,
                                       PlayerData.PassRatio,
                                       PlayerData.KillRatio,
                                       PlayerData.MeleeRatio,
                                       *ClusterLabel);

    FFileHelper::SaveStringToFile(DataLine, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

void APlayerBehaviorLearner::ExportAllDataToCSV()
{
    FString FilePath = FPaths::ProjectSavedDir() + TEXT("Logs/AIClusterResult.csv");

    // [변경됨] Persona 헤더 삭제
    FString Content = TEXT("VisitedNodeCount,PlayTime,PassRatio,KillRatio,MeleeRatio,ClusterID\n");

    for (int32 i = 0; i < PlayHistory.Num(); i++)
    {
        int32 ClusterIdx = FindNearestCentroid(PlayHistory[i]);

        Content += FString::Printf(TEXT("%f,%f,%f,%f,%f,%d\n"),
                                   PlayHistory[i].VisitedNodeCount,
                                   PlayHistory[i].PlayTime,
                                   PlayHistory[i].PassRatio,
                                   PlayHistory[i].KillRatio,
                                   PlayHistory[i].MeleeRatio,
                                   ClusterIdx);
    }

    Content += TEXT("\n# Centroids\n");
    for (int32 c = 0; c < Centroids.Num(); c++)
    {
        Content += FString::Printf(TEXT("%f,%f,%f,%f,%f,Centroid_%d\n"),
                                   Centroids[c].VisitedNodeCount,
                                   Centroids[c].PlayTime,
                                   Centroids[c].PassRatio,
                                   Centroids[c].KillRatio,
                                   Centroids[c].MeleeRatio,
                                   c);
    }

    FFileHelper::SaveStringToFile(Content, *FilePath);
    UE_LOG(LogTemp, Warning, TEXT("클러스터 결과 CSV 내보내기 완료: %s"), *FilePath);
}