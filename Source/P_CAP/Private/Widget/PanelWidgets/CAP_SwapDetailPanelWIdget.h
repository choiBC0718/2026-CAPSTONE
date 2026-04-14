// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_SwapDetailPanelWIdget.generated.h"

enum class EItemGrade : uint8;
/**
 * 
 */
UCLASS()
class UCAP_SwapDetailPanelWIdget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void UpdateDetailInfo(UObject* ItemData);

protected:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ItemNameText;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ItemGradeText;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ItemDescriptionText;
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* FeatureIconBox;

	FText GetGradeText(EItemGrade Grade) const;
	void AddFeatureIconToBox(TSoftObjectPtr<class UTexture2D> IconPtr);

	UPROPERTY(EditDefaultsOnly, Category="Setting")
	FVector2D SkillSynergyIconSize = FVector2D(125.f);
};
