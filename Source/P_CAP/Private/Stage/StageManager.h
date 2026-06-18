// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Map/Debug/MapManager.h"
#include "Stage/StageDataAsset.h"
#include "Stage/StageLoadingWidget.h"
#include "StageManager.generated.h"

UCLASS()
class AStageManager : public AActor
{
	GENERATED_BODY()

public:
	AStageManager();

	UFUNCTION(BlueprintCallable, Category="Stage")
	void StartStage(int32 StageIndex);

	UFUNCTION(BlueprintCallable, Category="Stage")
	void AdvanceToNextStage();

	UFUNCTION(BlueprintPure, Category="Stage")
	int32 GetCurrentStageIndex() const { return CurrentStageIndex; }

	UFUNCTION(BlueprintPure, Category="Stage")
	const UStageDataAsset* GetStageDataAsset() const { return StageDataAsset; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStageDataAsset> StageDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage", meta=(AllowPrivateAccess="true"))
	bool bStartFirstStageOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AMapManager> MapManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage|Loading", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UStageLoadingWidget> LoadingWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage|Loading", meta=(AllowPrivateAccess="true", ClampMin="0.1"))
	float LoadingTotalDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage|Loading", meta=(AllowPrivateAccess="true", ClampMin="0.0", ClampMax="1.0"))
	float StageGenerateProgressRatio = 0.4f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage", meta=(AllowPrivateAccess="true"))
	int32 CurrentStageIndex = INDEX_NONE;

	UPROPERTY(Transient)
	TObjectPtr<UStageLoadingWidget> ActiveLoadingWidget;

	int32 PendingStageIndex = INDEX_NONE;
	float LoadingElapsedTime = 0.f;
	bool bIsLoadingStage = false;
	bool bGeneratedPendingStage = false;

	AMapManager* ResolveMapManager();
	void StartStageWithLoading(int32 StageIndex);
	void ShowLoadingScreen();
	void HideLoadingScreen();
	void SetLoadingProgress(float Progress);
	void SetPlayerInputEnabled(bool bEnabled);
};
