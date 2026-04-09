// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_ItemSlotWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotMouseClick, class UCAP_ItemSlotWidget* ItemData);

UENUM(BlueprintType)
enum class ESlotItemType : uint8
{
	Weapon,
	Item
};
/**
 * InventoryTabWidget에 들어갈 패널(장착한 무기 / 아이템)의 요소
 */
UCLASS()
class UCAP_ItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	FOnSlotMouseClick OnLeftMouseClick;
	FOnSlotMouseClick OnRightMouseClick; 
	
	void InitSlot(ESlotItemType InSlotType, UTexture2D* InIcon, UObject* InItemData);
	void SetSlotNumber(int NewSlotNumber);

	UPROPERTY(BlueprintReadOnly)
	ESlotItemType SlotType;
	UPROPERTY(BlueprintReadOnly)
	UObject* SlotItemData = nullptr;

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* ItemIcon;

private:
	// WASD로 포커스 들어왔을 때 처리
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void RightButtonClicked();
	virtual void LeftButtonClicked();

	int SlotNumber;
};
