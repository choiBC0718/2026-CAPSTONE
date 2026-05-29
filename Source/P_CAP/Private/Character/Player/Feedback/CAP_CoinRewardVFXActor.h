#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAP_CoinRewardVFXActor.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FCAPCoinRewardFeedbackParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Coin Reward")
	TObjectPtr<UNiagaraSystem> RewardVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Coin Reward")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Coin Reward")
	FVector SourceLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Coin Reward")
	int32 CoinAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Coin Reward", meta=(ClampMin="0.0"))
	float AbsorbDelay = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Coin Reward", meta=(ClampMin="0.0"))
	float KillRadius = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Coin Reward", meta=(ClampMin="0.0"))
	float AbsorbSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Coin Reward", meta=(ClampMin="0.0"))
	float FindRadius = 5000.f;
};

UCLASS(BlueprintType, Blueprintable)
class ACAP_CoinRewardVFXActor : public AActor
{
	GENERATED_BODY()

public:
	ACAP_CoinRewardVFXActor();

	UFUNCTION(BlueprintCallable, Category="Coin Reward")
	void Play(const FCAPCoinRewardFeedbackParams& Params);

	UFUNCTION(BlueprintCallable, Category="Coin Reward")
	void PlayAtLocation(AActor* TargetActor, FVector SourceLocation, int32 CoinAmount, float AbsorbDelay = 0.f);

protected:
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void HandleVFXFinished(UNiagaraComponent* FinishedComponent);

	UPROPERTY(VisibleDefaultsOnly, Category="Coin Reward")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleDefaultsOnly, Category="Coin Reward")
	TObjectPtr<UNiagaraComponent> NiagaraComponent;

	UPROPERTY(EditDefaultsOnly, Category="Coin Reward")
	TObjectPtr<UNiagaraSystem> DefaultRewardVFX;

	UPROPERTY(Transient)
	FCAPCoinRewardFeedbackParams FeedbackParams;

	float AbsorbStartTime = 0.f;
	bool bFeedbackStarted = false;
};
