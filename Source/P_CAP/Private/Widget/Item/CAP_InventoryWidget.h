// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_ItemSlotWidget.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "CAP_InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_InventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	void RefreshLeftPanel(ACAP_PlayerCharacter* PlayerCharacter);
	void HandleInteractInput(bool bIsPressed);
protected:
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* WeaponBox;
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* ItemWrapBox;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UCAP_ItemSlotWidget> SlotWidgetClass;

private:
	UPROPERTY()
	UCAP_ItemSlotWidget* CurrentFocusedSlot = nullptr;
	
	UFUNCTION()
	void OnAnySlotFocused(UCAP_ItemSlotWidget* FocusedSlot);
};
