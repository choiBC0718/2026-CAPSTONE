// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_StatEnhanceDetailWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_StatEnhanceDetailWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateDetailInfo(const FText& EnhanceName, const FText& Description, int32 MaxLv, int32 CurrentLv);

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayName;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LevelText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DescriptionText;
};
