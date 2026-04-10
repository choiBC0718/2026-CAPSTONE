// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_ItemSwapWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_ItemSwapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	void InitSwapUI(class ACAP_PlayerCharacter* Player, class UCAP_ItemInstance* NewItemInst);
	void MoveSelection(FVector2D InputVal);
	
	void ConfirmSwap();

private:
	UPROPERTY(meta=(BindWidget))
	class UScrollBox* TopSynergyScrollBox;
	UPROPERTY(meta=(BindWidget))
	class UWrapBox* ItemWrapBox;
	UPROPERTY(meta=(BindWidget))
	class UCAP_SwapDetailPanelWIdget* OldItemDetailPanel;
	UPROPERTY(meta=(BindWidget))
	class UCAP_SwapDetailPanelWIdget* NewItemDetailPanel;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* InformationText;

	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<class UCAP_ItemSlotWidget> ItemSlotWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<class UCAP_SynergySimulSlotWidget> SynergySimulSlotClass;

	UPROPERTY()
	TArray<class UCAP_ItemSlotWidget*> ItemSlots;
	UPROPERTY()
	class UCAP_ItemSlotWidget* CurrentSelectedSlot;
	UPROPERTY()
	class UCAP_ItemInstance* NewItemToSwap;

	void HandleSlotLeftClicked(class UCAP_ItemSlotWidget* ClickedSlot);
	void InitNearbySlot();
	void UpdateTopSynergyIcons();

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FMargin ItemSlotMargin = FMargin(5.f);
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FVector2D IconSize = FVector2D(40.f);
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	float SynergyLvFontSize = 15.f;
	
};
