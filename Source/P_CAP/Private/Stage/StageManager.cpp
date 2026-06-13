// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/StageManager.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Map/Debug/MapManager.h"
#include "Map/SpecialRoomTransitionSubsystem.h"
#include "Stage/StageDataAsset.h"
#include "Stage/StageLoadingWidget.h"

AStageManager::AStageManager()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);
}

void AStageManager::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USpecialRoomTransitionSubsystem* TransitionSubsystem = GameInstance->GetSubsystem<USpecialRoomTransitionSubsystem>())
		{
			if (TransitionSubsystem->ShouldSkipStageAutoStart())
			{
				TransitionSubsystem->MarkStageAutoStartSkipped();
				return;
			}
		}
	}

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
	StartStageWithLoading(CurrentStageIndex + 1);
}

void AStageManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsLoadingStage)
	{
		return;
	}

	LoadingElapsedTime += DeltaSeconds;

	const float SafeTotalDuration = FMath::Max(0.1f, LoadingTotalDuration);
	const float Progress = FMath::Clamp(LoadingElapsedTime / SafeTotalDuration, 0.f, 1.f);
	SetLoadingProgress(Progress);

	if (!bGeneratedPendingStage && Progress >= StageGenerateProgressRatio)
	{
		bGeneratedPendingStage = true;
		StartStage(PendingStageIndex);
	}

	if (Progress >= 1.f)
	{
		bIsLoadingStage = false;
		PendingStageIndex = INDEX_NONE;
		HideLoadingScreen();
		SetPlayerInputEnabled(true);
		SetActorTickEnabled(false);
	}
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

void AStageManager::StartStageWithLoading(int32 StageIndex)
{
	if (bIsLoadingStage)
	{
		return;
	}

	if (!LoadingWidgetClass)
	{
		StartStage(StageIndex);
		return;
	}

	bIsLoadingStage = true;
	bGeneratedPendingStage = false;
	PendingStageIndex = StageIndex;
	LoadingElapsedTime = 0.f;

	SetPlayerInputEnabled(false);
	ShowLoadingScreen();
	SetLoadingProgress(0.f);
	SetActorTickEnabled(true);
}

void AStageManager::ShowLoadingScreen()
{
	if (ActiveLoadingWidget || !LoadingWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}

	ActiveLoadingWidget = CreateWidget<UStageLoadingWidget>(PlayerController, LoadingWidgetClass);
	if (ActiveLoadingWidget)
	{
		ActiveLoadingWidget->AddToViewport(1000);
	}
}

void AStageManager::HideLoadingScreen()
{
	if (ActiveLoadingWidget)
	{
		ActiveLoadingWidget->RemoveFromParent();
		ActiveLoadingWidget = nullptr;
	}
}

void AStageManager::SetLoadingProgress(float Progress)
{
	if (ActiveLoadingWidget)
	{
		ActiveLoadingWidget->SetLoadingProgress(Progress);
	}
}

void AStageManager::SetPlayerInputEnabled(bool bEnabled)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}

	PlayerController->SetIgnoreMoveInput(!bEnabled);
	PlayerController->SetIgnoreLookInput(!bEnabled);
}
