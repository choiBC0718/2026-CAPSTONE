#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerBehaviorLearner.generated.h"

UENUM(BlueprintType)
enum class EPlayerPersona : uint8
{
	Unknown,
	Explorer,
	SpeedRunner 
};

UENUM(BlueprintType)
enum class ELearnerMode : uint8
{
	Training_Explorer,
	Training_SpeedRunner,
	Inference
};

UCLASS()
class P_CAP_API APlayerBehaviorLearner : public AActor
{
	GENERATED_BODY()
    
public: 
	APlayerBehaviorLearner();

	UPROPERTY(EditAnywhere, Category="AI Learning")
	ELearnerMode CurrentMode = ELearnerMode::Inference;

	UPROPERTY(EditAnywhere, Category="AI Learning|Memory")
	FVector ExplorerCentroid;

	UPROPERTY(EditAnywhere, Category="AI Learning|Memory")
	FVector SpeedRunnerCentroid;

	// 3번째 매개변수를 float(비율)로 변경
	void ProcessPlayerData(int32 VisitedNodeCount, float PlayTime, float PassRatio);
    
protected:
	virtual void BeginPlay() override;

private:
	void UpdateCentroids(); 
	EPlayerPersona Classify(FVector NewPlayerData); 
};