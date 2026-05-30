// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/CAP_MenuInterface.h"
#include "CAP_StatisticDashboardWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_StatisticDashboardWidget : public UUserWidget, public ICAP_MenuInterface
{
	GENERATED_BODY()

public:
	
	virtual void NativeConstruct() override;

	virtual void NativeOpenMenu() override;
	virtual void NativeCloseMenu() override;
	virtual void HandleUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime) override;
	virtual FOnMenuClosedSignature& GetOnMenuClosedDelegate() override {return OnMenuClosed;}
	virtual void HandleChangeSelectedSlot(FVector2D InputVal) override {}
	
	UPROPERTY()
	FOnMenuClosedSignature OnMenuClosed;

protected:
	UPROPERTY(meta = (BindWidget))
	class UCAP_StatisicSlotWidget* PlayTimeSlot; 
	UPROPERTY(meta = (BindWidget))
	class UCAP_StatisicSlotWidget* DefeatedEnemySlot; 

	UPROPERTY(meta = (BindWidget))
	class UCAP_StatisicSlotWidget* TotalDamageSlot;
	UPROPERTY(meta = (BindWidget))
	class UCAP_StatisicSlotWidget* MaxDamageSlot;
	UPROPERTY(meta = (BindWidget))
	class UCAP_StatisicSlotWidget* TakenDamageSlot;
	UPROPERTY(meta = (BindWidget))
	class UCAP_StatisicSlotWidget* HealSlot;

	UPROPERTY(meta = (BindWidget))
	class UCAP_CurrencySlotWidget* GoldSlot;
	UPROPERTY(meta = (BindWidget))
	class UCAP_CurrencySlotWidget* WeaponMatSlot;
	UPROPERTY(meta = (BindWidget))
	class UCAP_CurrencySlotWidget* MagicStoneSlot;
	
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* EquipmentWrapBox;
	UPROPERTY(meta = (BindWidget))
	class UButton* ReturnBtn;
	UPROPERTY(meta = (BindWidget))
	class UImage* ReturnKeyIcon;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* OpenAnim;
	
	UFUNCTION()
	void OnReturnBtnClicked();
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCAP_ItemSlotWidget> ItemSlotClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FName VillageLevelName = "Map_Village";
	
private:
	void BringEquipments();
	void SetInteractionKey();
	
	FText FormatPlayTime(float TotalSeconds);
	FText FormatNumber(float Number);
};
