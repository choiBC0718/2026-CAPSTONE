// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_ItemSlotWidget.h"
#include "Blueprint/UserWidget.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_ItemDetailPanelWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_ItemDetailPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void UpdateDetailInfo(UObject* ItemData, ESlotItemType ItemType);
	
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GradeText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DescriptionText;
	
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* FeatureIconBox;

	FText GetGradeText(EItemGrade Grade) const;
};
