// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_SynergySimulSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_SynergySimulSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitSlot(class UTexture2D* Icon, int32 OriginalCount, int32 NewCount);
	
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* SynergyIcon;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CountText;
};
