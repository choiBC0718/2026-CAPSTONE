// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StageLoadingWidget.generated.h"

class UProgressBar;

UCLASS()
class UStageLoadingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Loading")
	void SetLoadingProgress(float InProgress);

protected:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> LoadingProgressBar;
};
