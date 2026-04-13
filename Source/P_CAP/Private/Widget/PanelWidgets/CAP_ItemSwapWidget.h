// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/CAP_MenuInterface.h"
#include "CAP_ItemSwapWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_ItemSwapWidget : public UUserWidget, public ICAP_MenuInterface
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
	
	void InitSwapUI(class ACAP_PlayerCharacter* Player, class UCAP_ItemInstance* NewItemInst);
	void MoveSelection(FVector2D InputVal);
	
	void ConfirmSwap();

	virtual void NativeOpenMenu() override;
	virtual void NativeCloseMenu() override;
	virtual FOnMenuClosedSignature& GetOnMenuClosedDelegate() override;

	UPROPERTY(BlueprintAssignable)
	FOnMenuClosedSignature OnMenuClosed;

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

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* SlideAnim;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* CloseAnim;

	
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
