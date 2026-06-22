// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_SynergyToolTipWidget.generated.h"

class UCAP_SynergyDataAsset;
/**
 * 
 */
UCLASS()
class UCAP_SynergyToolTipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupToolTip(UCAP_SynergyDataAsset* SynergyDA);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SynergyNameText;

	UPROPERTY(meta = (BindWidget))
	class URichTextBlock* SynergyEffectsText;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* SynergyIcon;
};
