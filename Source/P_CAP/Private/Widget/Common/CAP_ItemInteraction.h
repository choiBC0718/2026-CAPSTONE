// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_ItemInteraction.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_ItemInteraction : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetInteractionUIVisibility(bool bVisible);
	void UpdateInteractProgress(float Progress);
	void SetInteractKeyText(const FString& KeyName);
	
private:

	UPROPERTY(meta = (BindWidget))
	class UImage* InteractProgressImage;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InteractText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KeyText;

	UPROPERTY()
	class UMaterialInstanceDynamic* ProgressMID;
};
