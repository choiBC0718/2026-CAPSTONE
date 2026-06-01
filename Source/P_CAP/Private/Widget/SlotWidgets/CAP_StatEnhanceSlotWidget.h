// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_StatEnhanceSlotWidget.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnhanceSlotFocused, class UCAP_StatEnhanceSlotWidget* FocusedSlot);

/**
 * 
 */
UCLASS()
class UCAP_StatEnhanceSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void InitSlot(class ACAP_PlayerCharacter* Player);
	void SetSlotSelected(bool bIsSelected);
	void SetConfirmColor(FLinearColor Color);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Data", meta=(RowType="/Script/P_CAP.StatEnhanceTableRow"))
	FDataTableRowHandle StatEnhanceDataTableRow;
	
	FOnEnhanceSlotFocused OnEnhanceSlotFocused;

	UPROPERTY()
	class UCAP_StatEnhanceSlotWidget* UpSlot;
	UPROPERTY()
	class UCAP_StatEnhanceSlotWidget* DownSlot;
	UPROPERTY()
	class UCAP_StatEnhanceSlotWidget* LeftSlot;
	UPROPERTY()
	class UCAP_StatEnhanceSlotWidget* RightSlot;
	
protected:
	// 마우스 클릭했을 때 포커스 가져오도록 발동
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* FocusBorderImg;
	UPROPERTY(meta = (BindWidget))
	class UImage* StatIcon;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* LevelText;
};
