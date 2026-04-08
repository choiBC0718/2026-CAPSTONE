// Fill out your copyright notice in the Description page of Project Settings.

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
       UE_LOG(LogTemp, Warning, TEXT("Goal->데이터 정산을 시작"));

       float PlayTime = GetWorld()->GetTimeSeconds();

       AQuadtreeManager* Quadtree = Cast<AQuadtreeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AQuadtreeManager::StaticClass()));
       APlayerBehaviorLearner* Learner = Cast<APlayerBehaviorLearner>(UGameplayStatics::GetActorOfClass(GetWorld(), APlayerBehaviorLearner::StaticClass()));

       if (Quadtree && Learner)
       {
          // 1. 쿼드트리 노드 방문 데이터 수집 (X축 데이터)
          TArray<FQuadtreeNode*> AllNodes = Quadtree->GetAllLeafNodes();
          int32 VisitedCount = 0;
          for (FQuadtreeNode* Node : AllNodes)
          {
             if (Node->VisitCount > 0)
             {
                VisitedCount++;
             }
          }

          // 2. 플레이어 트래커에서 장애물 통과 데이터 수집
          int32 PassedObstacles = 0;
          int32 AvoidedObstacles = 0;
          UPlayerTrackerComponent* Tracker = OtherActor->FindComponentByClass<UPlayerTrackerComponent>();
          
          if (Tracker)
          {
             PassedObstacles = Tracker->PassedObstacleCount;
             AvoidedObstacles = Tracker->AvoidedObstacleCount;
          }

          // [추가된 핵심 로직] 돌파율(%) 계산
          int32 TotalEncountered = PassedObstacles + AvoidedObstacles;
          float PassRatio = 0.0f;
          
          if (TotalEncountered > 0)
          {
              PassRatio = (static_cast<float>(PassedObstacles) / TotalEncountered) * 100.0f;
          }

          UE_LOG(LogTemp, Log, TEXT("수집 결과 -> 밟은 구역: %d개 / 소요 시간: %f초 / 만난 장애물: %d개 / 돌파율: %f%%"), VisitedCount, PlayTime, TotalEncountered, PassRatio);

          // 3. AI에게 비율 데이터(Z축) 전달
          Learner->ProcessPlayerData(VisitedCount, PlayTime, PassRatio);
       }
       else
       {
          UE_LOG(LogTemp, Error, TEXT("맵에 QuadtreeManager OR PlayerBehaviorLearner X"));
       }
    }
}