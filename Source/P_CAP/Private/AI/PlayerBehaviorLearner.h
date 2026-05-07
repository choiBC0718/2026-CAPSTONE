#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/FileHelper.h" 
#include "Misc/Paths.h"
#include "PlayerBehaviorLearner.generated.h"

// [변경됨] EPlayerPersona 열거형 삭제 완료

// [변경됨] 맵 팀에게 전달할 유저 성향 데이터 구조체 신설 (0.0 ~ 1.0)
USTRUCT(BlueprintType)
struct FPlayerTendencyModifier
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI Tendency")
	float ExplorationRate;  // 탐색을 좋아하는 정도 (방문율+시간 기반)

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI Tendency")
	float CombatAggression; // 전투를 좋아하는 정도 (처치율)

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI Tendency")
	float MeleePreference;  // 근접전을 선호하는 정도

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI Tendency")
	float ObstacleBypass;   // 장애물을 피하지 않고 돌파하는 정도

	FPlayerTendencyModifier()
	{
		// 기본값은 중간(0.5) 난이도로 설정 (콜드 스타트 방지)
		ExplorationRate = 0.5f;
		CombatAggression = 0.5f;
		MeleePreference = 0.5f;
		ObstacleBypass = 0.5f;
	}
};

USTRUCT(BlueprintType)
struct FPlayerBehaviorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Data")
	float VisitedNodeCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Data")
	float PlayTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Data")
	float PassRatio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Data")
	float KillRatio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Data")
	float MeleeRatio;

	FPlayerBehaviorData()
	{
		VisitedNodeCount = 0.f;
		PlayTime = 0.f;
		PassRatio = 0.f;
		KillRatio = 0.f;
		MeleeRatio = 0.f;
	}

	FPlayerBehaviorData operator+(const FPlayerBehaviorData& Other) const
	{
		FPlayerBehaviorData Result;
		Result.VisitedNodeCount = VisitedNodeCount + Other.VisitedNodeCount;
		Result.PlayTime = PlayTime + Other.PlayTime;
		Result.PassRatio = PassRatio + Other.PassRatio;
		Result.KillRatio = KillRatio + Other.KillRatio;
		Result.MeleeRatio = MeleeRatio + Other.MeleeRatio;
		return Result;
	}

	FPlayerBehaviorData operator/(float Divisor) const
	{
		FPlayerBehaviorData Result;
		if (Divisor == 0.f) return *this;
		Result.VisitedNodeCount = VisitedNodeCount / Divisor;
		Result.PlayTime = PlayTime / Divisor;
		Result.PassRatio = PassRatio / Divisor;
		Result.KillRatio = KillRatio / Divisor;
		Result.MeleeRatio = MeleeRatio / Divisor;
		return Result;
	}

	FPlayerBehaviorData Lerp(const FPlayerBehaviorData& Target, float Alpha) const
	{
		FPlayerBehaviorData Result;
		Result.VisitedNodeCount = FMath::Lerp(VisitedNodeCount, Target.VisitedNodeCount, Alpha);
		Result.PlayTime = FMath::Lerp(PlayTime, Target.PlayTime, Alpha);
		Result.PassRatio = FMath::Lerp(PassRatio, Target.PassRatio, Alpha);
		Result.KillRatio = FMath::Lerp(KillRatio, Target.KillRatio, Alpha);
		Result.MeleeRatio = FMath::Lerp(MeleeRatio, Target.MeleeRatio, Alpha);
		return Result;
	}

	static float Distance(const FPlayerBehaviorData& A, const FPlayerBehaviorData& B)
	{
		float DistSq = FMath::Square(A.VisitedNodeCount - B.VisitedNodeCount) +
					   FMath::Square(A.PlayTime - B.PlayTime) +
					   FMath::Square(A.PassRatio - B.PassRatio) +
					   FMath::Square(A.KillRatio - B.KillRatio) +
					   FMath::Square(A.MeleeRatio - B.MeleeRatio);
		return FMath::Sqrt(DistSq);
	}
};

UCLASS()
class P_CAP_API APlayerBehaviorLearner : public AActor
{
	GENERATED_BODY()
    
public: 
	APlayerBehaviorLearner();

	// =============================================
	// K-Means 설정
	// =============================================
	UPROPERTY(EditAnywhere, Category="AI Learning|K-Means", meta=(ClampMin="2", ClampMax="5"))
	int32 K = 2;

	UPROPERTY(EditAnywhere, Category="AI Learning|K-Means", meta=(ClampMin="1", ClampMax="100"))
	int32 MaxIterations = 30;

	UPROPERTY(EditAnywhere, Category="AI Learning|K-Means", meta=(ClampMin="2"))
	int32 MinDataForClustering = 4;

	UPROPERTY(EditAnywhere, Category="AI Learning|K-Means", meta=(ClampMin="10"))
	int32 MaxHistorySize = 100;

	UPROPERTY(EditAnywhere, Category="AI Learning|K-Means", meta=(ClampMin="0.0001", ClampMax="0.01"))
	float ConvergenceThreshold = 0.001f;

	// =============================================
	// K-Means 결과
	// =============================================
	UPROPERTY(VisibleAnywhere, Category="AI Learning|Data")
	TArray<FPlayerBehaviorData> PlayHistory;

	UPROPERTY(VisibleAnywhere, Category="AI Learning|Result")
	TArray<FPlayerBehaviorData> Centroids;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Learning|Result")
	bool bIsClusteringReady = false;

	// =============================================
	// 외부 인터페이스
	// =============================================
	void ProcessPlayerData(FPlayerBehaviorData PlayerDataPoint);

	// [변경됨] 맵 생성기에서 호출할 최종 데이터 추출 함수
	UFUNCTION(BlueprintCallable, Category="AI Learning")
	FPlayerTendencyModifier GetCurrentPlayerTendency();

protected:
	virtual void BeginPlay() override;

private:
	void RunKMeans();
	void InitializeCentroidsKMeansPP(); 
	int32 FindNearestCentroid(const FPlayerBehaviorData& DataPoint) const;
	
	// [변경됨] MapCentroidsToPersonas() 삭제 완료

	void SaveLearnerData();
	void LoadLearnerData();

	void ExportDataToCSV(const FPlayerBehaviorData& PlayerData, const FString& ClusterLabel);
	void ExportAllDataToCSV();
};