// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Library/CAP_WidgetHelper.h"
#include "Interface/CAP_MenuInterface.h"
#include "CAP_SkillRerollPanelWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_SkillRerollPanelWidget : public UUserWidget, public ICAP_MenuInterface
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeOpenMenu() override;
	virtual void NativeCloseMenu() override;
	virtual FOnMenuClosedSignature& GetOnMenuClosedDelegate() override {return OnMenuClosed;}
	virtual void HandleChangeSelectedSlot(FVector2D InputVal) override;
	virtual void HandleUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime = 0) override;
	
	FOnMenuClosedSignature OnMenuClosed;

protected:
	UFUNCTION()
	void OnRerollClicked();
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
	class UCAP_CurrencySlotWidget* GoldMatCurrency;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* OpenAnim;

	bool bIsConfirmMode = false;
	int32 CurrentButtonIndex = -1;
	void SetConfirmMode(bool bIsConfirm);
	void RefreshButtonVisuals();
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FButtonVisualSettings ButtonSettings;
	
	UPROPERTY()
	class ANPC_SkillReroll* OwnerNPC;

	UPROPERTY()
	class ACAP_PlayerCharacter* CachedPlayer;
};
