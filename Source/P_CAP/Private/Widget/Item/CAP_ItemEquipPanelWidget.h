// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_ItemEquipPanelWidget.generated.h"

class ACAP_PlayerCharacter;
class UCAP_ItemSlotWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPanelSlotFocused, UCAP_ItemSlotWidget*, FocusedSlot);
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

	UPROPERTY()
	FOnPanelSlotFocused OnPanelSlotFocused;
	
private:
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* WeaponList;
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* PassiveItemList;

	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	TSubclassOf<UCAP_ItemSlotWidget> ItemSlotWidgetClass;

	UPROPERTY()
	class UCAP_InventoryComponent* InventoryComponent;

	TArray<UCAP_ItemSlotWidget*> WeaponSlots;
	TArray<UCAP_ItemSlotWidget*> ItemSlots;

	// 개별 슬롯 포커스 이벤트를 받아 부모로 토스
	UFUNCTION()
	void HandleSlotFocused(UCAP_ItemSlotWidget* FocusedSlot);
};
