#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LearnerSaveGame.generated.h"

UCLASS()
class ULearnerSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	// FVector2D -> FVector로 변경
	UPROPERTY(VisibleAnywhere, Category = "AI Data")
	FVector SavedExplorerCentroid;

	UPROPERTY(VisibleAnywhere, Category = "AI Data")
	FVector SavedSpeedRunnerCentroid;
};