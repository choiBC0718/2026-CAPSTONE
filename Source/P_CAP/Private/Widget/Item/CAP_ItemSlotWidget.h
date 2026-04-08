// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_ItemSlotWidget.generated.h"

// 슬롯이 포커스되거나 클릭되었을 때 인벤토리에 알리기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotFocused, class UCAP_ItemSlotWidget*, FocusedSlot);

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

	void InitSlot(ESlotItemType InSlotType, UTexture2D* InIcon, UObject* InItemData);
	void SetSlotNumber(int NewSlotNumber);
	FOnSlotFocused OnSlotFocused;

	UPROPERTY(BlueprintReadOnly)
	ESlotItemType SlotType;
	UPROPERTY(BlueprintReadOnly)
	UObject* SlotItemData = nullptr;

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* SlotButton;
	UPROPERTY(meta = (BindWidget))
	class UImage* ItemIcon;

private:
	UFUNCTION()
	void HandleSlotClicked();

	// WASD로 포커스 들어왔을 때 처리
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;

	int SlotNumber;
};
