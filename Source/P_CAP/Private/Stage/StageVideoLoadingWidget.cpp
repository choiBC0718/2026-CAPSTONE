#include "Stage/StageVideoLoadingWidget.h"

void UStageVideoLoadingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (LoadingVideoImage && LoadingMediaTexture)
	{
		FSlateBrush VideoBrush;
		VideoBrush.SetResourceObject(LoadingMediaTexture);
		VideoBrush.ImageSize = FVector2D(1920.f, 1080.f);
		LoadingVideoImage->SetBrush(VideoBrush);
	}

	if (LoadingMediaPlayer && LoadingMediaSource)
	{
		LoadingMediaPlayer->SetLooping(true);
		LoadingMediaPlayer->PlayOnOpen = true;
		LoadingMediaPlayer->OpenSource(LoadingMediaSource);
	}
}

void UStageVideoLoadingWidget::NativeDestruct()
{
	if (LoadingMediaPlayer)
	{
		LoadingMediaPlayer->Close();
	}

	Super::NativeDestruct();
}
