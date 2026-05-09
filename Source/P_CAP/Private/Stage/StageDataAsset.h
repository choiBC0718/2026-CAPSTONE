// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StageDataAsset.generated.h"

class URoomMonsterSpawnDataAsset;

USTRUCT(BlueprintType)
struct FStageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage")
	FName StageId = TEXT("Stage_01");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage|Map", meta=(ClampMin="1"))
	int32 RoomCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage|Map")
	bool bUseRandomSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage|Map")
	int32 FixedSeed = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage|Monster")
	TObjectPtr<URoomMonsterSpawnDataAsset> MonsterSpawnDataAsset;
};

UCLASS(BlueprintType)
class UStageDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FStageConfig* GetStageConfig(int32 StageIndex) const;
	int32 GetStageCount() const { return Stages.Num(); }
	bool IsValidStageIndex(int32 StageIndex) const { return Stages.IsValidIndex(StageIndex); }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage", meta=(AllowPrivateAccess="true"))
	TArray<FStageConfig> Stages;
};
