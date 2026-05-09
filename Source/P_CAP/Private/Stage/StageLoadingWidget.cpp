// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/StageLoadingWidget.h"

#include "Components/ProgressBar.h"

void UStageLoadingWidget::SetLoadingProgress(float InProgress)
{
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(FMath::Clamp(InProgress, 0.f, 1.f));
	}
}
