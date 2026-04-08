#include "AI/PlayerBehaviorLearner.h" // 본인의 폴더 경로에 맞게 수정 필요 (예: "PlayerBehaviorLearner.h")
#include "Kismet/GameplayStatics.h"
#include "AI/LearnerSaveGame.h"       // 본인의 폴더 경로에 맞게 수정 필요 (예: "LearnerSaveGame.h")

APlayerBehaviorLearner::APlayerBehaviorLearner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APlayerBehaviorLearner::BeginPlay()
{
    Super::BeginPlay();

    if (UGameplayStatics::DoesSaveGameExist(TEXT("AIBrainData"), 0))
    {
       USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(TEXT("AIBrainData"), 0);
       ULearnerSaveGame* LoadGameInstance = Cast<ULearnerSaveGame>(LoadedGame);
       
       if (LoadGameInstance)
       {
          ExplorerCentroid = LoadGameInstance->SavedExplorerCentroid;
          SpeedRunnerCentroid = LoadGameInstance->SavedSpeedRunnerCentroid;
          UE_LOG(LogTemp, Warning, TEXT("세이브 파일에서 데이터 불러옴"));
       }
       else
       {
          UE_LOG(LogTemp, Error, TEXT("파일은 있지만 ULearnerSaveGame으로 변환 실패"));
       }
    }
    else
    {
       UE_LOG(LogTemp, Warning, TEXT("세이브 파일 없음. 초기값으로 시작"));
    }

    if (GEngine)
    {
       FString Msg = FString::Printf(TEXT("시작 뇌 수치 -> 탐험: %s | 직진: %s"), *ExplorerCentroid.ToString(), *SpeedRunnerCentroid.ToString());
       GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, Msg);
    }
}

// 3번째 매개변수를 float PassRatio로 변경하여 헤더와 일치시킴
void APlayerBehaviorLearner::ProcessPlayerData(int32 VisitedNodeCount, float PlayTime, float PassRatio)
{
    // Z축에 PassRatio (float) 할당
    FVector PlayerDataPoint(VisitedNodeCount, PlayTime, PassRatio);

    if (CurrentMode == ELearnerMode::Training_Explorer)
    {
       ExplorerCentroid = FMath::Lerp(ExplorerCentroid, PlayerDataPoint, 0.3f);
       UpdateCentroids(); 
       UE_LOG(LogTemp, Warning, TEXT("탐험형 뇌 업데이트 완료: %s"), *ExplorerCentroid.ToString());
    }
    else if (CurrentMode == ELearnerMode::Training_SpeedRunner)
    {
       SpeedRunnerCentroid = FMath::Lerp(SpeedRunnerCentroid, PlayerDataPoint, 0.3f);
       UpdateCentroids(); 
       UE_LOG(LogTemp, Warning, TEXT("직진형 뇌 업데이트 완료: %s"), *SpeedRunnerCentroid.ToString());
    }
    else if (CurrentMode == ELearnerMode::Inference)
    {
       EPlayerPersona Result = Classify(PlayerDataPoint);
       FString PersonaName = (Result == EPlayerPersona::Explorer) ? TEXT("탐험형(Explorer)") : TEXT("직진형(SpeedRunner)");
       UE_LOG(LogTemp, Error, TEXT("AI 판별 결과 - 이 유저는 [%s] 입니다!"), *PersonaName);
    }
}

void APlayerBehaviorLearner::UpdateCentroids()
{
    USaveGame* SaveGameObj = UGameplayStatics::CreateSaveGameObject(ULearnerSaveGame::StaticClass());
    ULearnerSaveGame* SaveGameInstance = Cast<ULearnerSaveGame>(SaveGameObj);
    
    if (SaveGameInstance)
    {
       SaveGameInstance->SavedExplorerCentroid = ExplorerCentroid;
       SaveGameInstance->SavedSpeedRunnerCentroid = SpeedRunnerCentroid;
       
       bool bIsSaved = UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("AIBrainData"), 0);
       
       if (bIsSaved && GEngine)
       {
           FString Msg = FString::Printf(TEXT("뇌 영구 저장 완료! 탐험: %s | 직진: %s"), *ExplorerCentroid.ToString(), *SpeedRunnerCentroid.ToString());
           GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, Msg);
       }
    }
}

EPlayerPersona APlayerBehaviorLearner::Classify(FVector NewPlayerData)
{
    float DistToExplorer = FVector::Distance(NewPlayerData, ExplorerCentroid);
    float DistToSpeedRunner = FVector::Distance(NewPlayerData, SpeedRunnerCentroid);

    UE_LOG(LogTemp, Log, TEXT(">> 탐험형 기준점과의 거리: %f"), DistToExplorer);
    UE_LOG(LogTemp, Log, TEXT(">> 직진형 기준점과의 거리: %f"), DistToSpeedRunner);

    if (DistToExplorer < DistToSpeedRunner)
    {
       return EPlayerPersona::Explorer;
    }
    else
    {
       return EPlayerPersona::SpeedRunner;
    }
}