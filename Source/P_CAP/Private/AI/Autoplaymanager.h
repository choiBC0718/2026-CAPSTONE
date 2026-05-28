#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutoPlayManager.generated.h"

class APlayerBehaviorLearner;
class ABotPlayController;

UCLASS()
class P_CAP_API AAutoPlayManager : public AActor
{
	GENERATED_BODY()

public:
	AAutoPlayManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoPlay")
	bool bAutoPlayEnabled = true;

	UPROPERTY(EditAnywhere, Category = "AutoPlay", meta=(ClampMin="1"))
	int32 TotalAutoRuns = 20;

	UPROPERTY(EditAnywhere, Category = "AutoPlay", meta=(ClampMin="0.5"))
	float RestartDelay = 2.0f;

	// 봇 플레이 속도 배율 (3.0 = 3배속, 5.0 = 5배속)
	UPROPERTY(EditAnywhere, Category = "AutoPlay", meta=(ClampMin="1.0", ClampMax="10.0"))
	float GameSpeed = 30.0f;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	APlayerBehaviorLearner* CachedLearner;

	UPROPERTY()
	ABotPlayController* ActiveBot;

	int32 LastKnownHistoryCount;
	FTimerHandle CompletionCheckTimer;

	void StartBotRun();
	void CheckRunCompletion();
	void ReloadLevel();
};