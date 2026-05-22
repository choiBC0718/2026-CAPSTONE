// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/CAP_MenuInterface.h"
#include "CAP_StatEnhancePanelWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_StatEnhancePanelWidget : public UUserWidget, public ICAP_MenuInterface
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

private:
	// 5*3 격자 그리드 패널
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* SlotWrapBox;
	UPROPERTY(meta = (BindWidget))
	class UCAP_StatEnhanceDetailWidget* DetailWidget;
	
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
	class UCAP_CurrencySlotWidget* MagicStoneCurrency;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* OpenAnim;

	UPROPERTY(EditDefaultsOnly, Category="Setup")
	TSubclassOf<class UCAP_StatEnhanceSlotWidget> SlotWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	class UDataTable* EnhanceDataTable;

	// 동적으로 생성된 슬롯 넣을 배열
	UPROPERTY()
	TArray<class UCAP_StatEnhanceSlotWidget*> CreatedSlots;
	UPROPERTY()
	class UCAP_StatEnhanceSlotWidget* CurrentSelectedSlot;
	
	UFUNCTION()
	void HandleSlotFocused(UCAP_StatEnhanceSlotWidget* FocusedSlot);
	UFUNCTION()
	void OnInnerEnhanceClicked();
	UFUNCTION()
	void OnInnerCloseClicked();
	UFUNCTION()
	void OnButtonHovered();

	void InitNearbySlot();
	void SetConfirmMode(bool bIsConfirm);
	void RefreshButtonVisuals();

	bool TryGetEnhanceData(class UCAP_StatEnhanceSlotWidget* SlotWidget, struct FStatEnhanceTableRow*& OutRowData, class ACAP_PlayerCharacter*& OutPlayer, FName& OutRowName, int32& OutCurrentLevel);
	FText GetFormattedDescription(const struct FStatEnhanceTableRow* RowData, int32 CurrentLevel);
	
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

	// 탐색 모드 대사
	UPROPERTY(EditDefaultsOnly, Category="Dialogue", meta=(MultiLine="true"))
	TArray<FText> BrowseDialoguePool;
	// 결정 상태 시 대사
	UPROPERTY(EditDefaultsOnly, Category="Dialogue", meta=(MultiLine="true"))
	TArray<FText> ConfirmDialoguePool;
	// 강화 성공 시 대사
	UPROPERTY(EditDefaultsOnly, Category="Dialogue", meta=(MultiLine="true"))
	TArray<FText> SuccessDialoguePool;
	// 강화 실패 시 대사
	UPROPERTY(EditDefaultsOnly, Category="Dialogue", meta=(MultiLine="true"))
	TArray<FText> FailDialoguePool;
	// 이미 최대일때 대사
	UPROPERTY(EditDefaultsOnly, Category="Dialogue", meta=(MultiLine="true"))
	TArray<FText> OnMaxLevelDialoguePool;

	void ApplyRandomDialogue(const TArray<FText>& DialoguePool, int32 Cost = -1);
};
