#pragma once

#include "CoreMinimal.h"
#include "Stage/StageLoadingWidget.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "MediaTexture.h"
#include "Components/Image.h"
#include "StageVideoLoadingWidget.generated.h"

/** Loading screen variant that plays a full-screen video during stage transitions. */
UCLASS()
class UStageVideoLoadingWidget : public UStageLoadingWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> LoadingVideoImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loading|Video")
	TObjectPtr<UMediaPlayer> LoadingMediaPlayer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loading|Video")
	TObjectPtr<UMediaSource> LoadingMediaSource;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loading|Video")
	TObjectPtr<UMediaTexture> LoadingMediaTexture;

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
};
