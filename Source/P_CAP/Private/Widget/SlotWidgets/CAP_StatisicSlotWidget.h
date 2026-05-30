// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_StatisicSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_StatisicSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void SetStatisticData(const FText& InValue);
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TitleText;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ValueText;

	UPROPERTY(EditAnywhere, Category="Title")
	FText StatisticTitle;
};
