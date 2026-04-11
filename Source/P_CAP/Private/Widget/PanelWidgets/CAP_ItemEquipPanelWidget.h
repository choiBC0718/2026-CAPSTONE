// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/SlotWidgets/CAP_ItemSlotWidget.h"
#include "Blueprint/UserWidget.h"
#include "CAP_ItemEquipPanelWidget.generated.h"

class ACAP_PlayerCharacter;
class UCAP_ItemSlotWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquipPanelSlotClicked, class UCAP_ItemSlotWidget*);
/**
 * 
 */
UCLASS()
class UCAP_ItemEquipPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 위젯 활성화시 데이터 갱신
	void RefreshPanel(ACAP_PlayerCharacter* PlayerCharacter);
	void MoveSelection(FVector2D InputVal);
	
	FOnEquipPanelSlotClicked OnPanelSlotClicked;
	
private:
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* WeaponList;
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* PassiveItemList;

	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	TSubclassOf<UCAP_ItemSlotWidget> ItemSlotWidgetClass;
	
	UPROPERTY()
	class UCAP_ItemSlotWidget* CurrentSelectedSlot;

	TArray<UCAP_ItemSlotWidget*> WeaponSlots;
	TArray<UCAP_ItemSlotWidget*> ItemSlots;

	UFUNCTION()
	void HandleSlotLeftClicked(class UCAP_ItemSlotWidget* ClickedSlot);
	UFUNCTION()
	void HandleSlotRightClicked(class UCAP_ItemSlotWidget* ClickedSlot);

	void CreateAndAddSlot(UWrapBox* TargetBox, TArray<UCAP_ItemSlotWidget*>& TargetArray, ESlotItemType SlotType, int32 Index, UObject* ItemData, UTexture2D* Icon);
	void InitNearbySlot();
};
