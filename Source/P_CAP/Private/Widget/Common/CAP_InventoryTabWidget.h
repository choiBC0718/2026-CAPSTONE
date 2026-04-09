// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "CAP_InventoryTabWidget.generated.h"

/**
 * Character Menu Widget에 들어갈 위젯
 * 시너지 정보, 장착한 아이템, 아이템 정보 등의 패널들의 부모 위젯
 */
UCLASS()
class UCAP_InventoryTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	void RefreshInventoryTab(ACAP_PlayerCharacter* PlayerCharacter);
	void NavigationInput(FVector2D InputVal);
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UCAP_ItemEquipPanelWidget* ItemEquipPanel;
	UPROPERTY(meta = (BindWidget))
	class UCAP_ItemDetailPanelWidget* ItemDetailPanel;
	UPROPERTY(meta = (BindWidget))
	class UCAP_ItemSynergyPanelWidget* ItemSynergyPanel;

private:
	void OnItemSlotClicked(class UCAP_ItemSlotWidget* ClickedSlot);
};
