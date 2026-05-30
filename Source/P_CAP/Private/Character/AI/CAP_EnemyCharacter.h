// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CAP_Character.h"
#include "Data/CAP_MonsterRewardDataAsset.h"
#include "CAP_EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_EnemyCharacter : public ACAP_Character
{
	GENERATED_BODY()

public:
	ACAP_EnemyCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category="Enemy|AI")
	void SetEnemyAIEnabled(bool bEnabled, AActor* TargetActor = nullptr);

	UFUNCTION(BlueprintPure, Category="Enemy|AI")
	bool IsEnemyAIEnabled() const { return bEnemyAIEnabled; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Enemy|Room")
	void OnRoomActivated(AActor* TargetActor);
	virtual void OnRoomActivated_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Enemy|Room")
	void OnRoomDeactivated();
	virtual void OnRoomDeactivated_Implementation();

	virtual void UpdateStackUI(const FGameplayTag& BehaviorTag, int32 CurrentStack, int32 MaxStack) override;
protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnDead() override;
	virtual void OnDeathMontageFinished() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy|UI")
	TObjectPtr<class UWidgetComponent> HealthBarWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|UI")
	TSubclassOf<class UCAP_OverheadStatsGauge> HealthBarWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Reward")
	TSubclassOf<class ACAP_CoinRewardVFXActor> CoinRewardVFXActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Reward")
	TObjectPtr<class UNiagaraSystem> CoinRewardVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Reward", meta=(ClampMin="0"))
	int32 CoinRewardAmount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Reward", meta=(ClampMin="0.0"))
	float CoinRewardAbsorbDelay = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Reward", meta=(ClampMin="0.0"))
	float CoinRewardKillRadius = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Reward", meta=(ClampMin="0.0"))
	float CoinRewardAbsorbSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Reward", meta=(ClampMin="0.0"))
	float CoinRewardFindRadius = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Reward")
	FVector CoinRewardSpawnOffset = FVector(0.f, 0.f, 45.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Reward|Currency")
	ECAPMonsterRewardGroup RewardGroup = ECAPMonsterRewardGroup::Normal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Death Dissolve")
	TObjectPtr<class UMaterialInterface> DeathDissolveMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Death Dissolve")
	FName DeathDissolveParameterName = TEXT("DissolveAmount");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Death Dissolve", meta=(ClampMin="0.0"))
	float DeathDissolveDuration = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Death Dissolve")
	float DeathDissolveStartValue = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Death Dissolve")
	float DeathDissolveEndValue = 1.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Enemy|AI")
	bool bEnemyAIEnabled = false;

private:
	void InitializeHealthBarWidget();
	void SpawnCoinRewardVFX();
	void GiveDeathCurrencyReward();
	void StartDeathDissolve();
	void FinishDeathDissolve();

	UPROPERTY()
	TObjectPtr<AActor> CurrentTargetActor;

	bool bDeathCurrencyRewardGranted = false;

	UPROPERTY()
	TArray<TObjectPtr<class UMaterialInstanceDynamic>> DeathDissolveDynamicMaterials;

	bool bPlayingDeathDissolve = false;
	float DeathDissolveElapsedTime = 0.f;
};
