// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/CAP_MenuInterface.h"
#include "CAP_WeaponEnhancePanelWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_WeaponEnhancePanelWidget : public UUserWidget, public ICAP_MenuInterface
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeOpenMenu() override;
	virtual void NativeCloseMenu() override;
	virtual FOnMenuClosedSignature& GetOnMenuClosedDelegate() override {return OnMenuClosed;}
	virtual void HandleChangeSelectedSlot(FVector2D InputVal) override;
	virtual void HandleUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime = 0) override;

	UPROPERTY()
	FOnMenuClosedSignature OnMenuClosed;

protected:
	UFUNCTION()
	void OnEnhanceClicked();
	UFUNCTION()
	void OnCloseClicked();
	UFUNCTION()
	void OnBtnHovered();
	
private:
	UPROPERTY(meta = (BindWidget))
	class UButton* InnerEnhanceBtn;
	UPROPERTY(meta = (BindWidget))
	class UButton* InnerCloseBtn;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DialogueText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InnerEnhanceText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InnerCloseText;
	UPROPERTY(meta = (BindWidget))
	class UCAP_CurrencySlotWidget* WeaponMatCurrency;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* OpenAnim;

	bool bIsConfirmMode = false;
	int32 CurrentButtonIndex = -1;
	
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FLinearColor ButtonHoverColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FLinearColor ButtonNormalColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FLinearColor ButtonHoverOutlineColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	float ButtonHoverOutlineWidth = 4.f;

	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	int32 NormalFontSize = 30;

	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	int32 HoverFontSize = 38;
	
	void SetConfirmMode(bool bIsConfirm);
	void RefreshButtonVisuals();

	UPROPERTY()
	class ANPC_WeaponEnhance* OwnerNPC;

	UPROPERTY()
	class ACAP_PlayerCharacter* CachedPlayer;
};
