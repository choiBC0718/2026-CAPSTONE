#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AI/PlayerBehaviorLearner.h"
#include "LearnerSaveGame.generated.h"

UCLASS()
class ULearnerSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	// 누적된 플레이 히스토리 (K-Means 입력 데이터)
	UPROPERTY(VisibleAnywhere, Category = "AI Data")
	TArray<FPlayerBehaviorData> SavedPlayHistory;

	// K-Means 산출 센트로이드
	UPROPERTY(VisibleAnywhere, Category = "AI Data")
	TArray<FPlayerBehaviorData> SavedCentroids;

	// [변경됨] 페르소나 저장 변수(SavedCentroidPersonas) 삭제 완료

	// 클러스터링 완료 여부
	UPROPERTY(VisibleAnywhere, Category = "AI Data")
	bool bSavedClusteringReady = false;
};