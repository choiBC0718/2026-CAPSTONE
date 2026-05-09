// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StageManager.generated.h"

class AMapManager;
class UStageDataAsset;

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

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStageDataAsset> StageDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage", meta=(AllowPrivateAccess="true"))
	bool bStartFirstStageOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AMapManager> MapManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage", meta=(AllowPrivateAccess="true"))
	int32 CurrentStageIndex = INDEX_NONE;

	AMapManager* ResolveMapManager();
};
