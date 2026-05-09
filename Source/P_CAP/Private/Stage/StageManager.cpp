// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/StageManager.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Map/Debug/MapManager.h"
#include "Stage/StageDataAsset.h"

AStageManager::AStageManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStageManager::BeginPlay()
{
	Super::BeginPlay();

	if (bStartFirstStageOnBeginPlay && StageDataAsset && StageDataAsset->GetStageCount() > 0)
	{
		StartStage(0);
	}
}

void AStageManager::StartStage(int32 StageIndex)
{
	if (!StageDataAsset)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("StageManager: StageDataAsset is missing"));
		}
		return;
	}

	const FStageConfig* StageConfig = StageDataAsset->GetStageConfig(StageIndex);
	if (!StageConfig)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::Red,
				FString::Printf(TEXT("StageManager: invalid stage index %d"), StageIndex)
			);
		}
		return;
	}

	if (AMapManager* ResolvedMapManager = ResolveMapManager())
	{
		CurrentStageIndex = StageIndex;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.0f,
				FColor::Green,
				FString::Printf(TEXT("Start Stage %d: %s"), StageIndex, *StageConfig->StageId.ToString())
			);
		}
		ResolvedMapManager->GenerateStage(*StageConfig);
	}
	else if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("StageManager: MapManager not found"));
	}
}

void AStageManager::AdvanceToNextStage()
{
	StartStage(CurrentStageIndex + 1);
}

AMapManager* AStageManager::ResolveMapManager()
{
	if (MapManager)
	{
		return MapManager;
	}

	MapManager = Cast<AMapManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapManager::StaticClass()));
	return MapManager;
}
