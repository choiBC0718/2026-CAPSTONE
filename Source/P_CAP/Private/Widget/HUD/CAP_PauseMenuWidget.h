// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Framework/Library/CAP_WidgetHelper.h"
#include "Interface/CAP_MenuInterface.h"
#include "CAP_PauseMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_PauseMenuWidget : public UUserWidget, public ICAP_MenuInterface
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	// ICAP_MenuInterface 구현
	virtual void NativeOpenMenu() override;
	virtual void NativeCloseMenu() override;
	virtual FOnMenuClosedSignature& GetOnMenuClosedDelegate() override { return OnMenuClosedDelegate; }
	virtual void HandleUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime = 0.f) override;
	virtual void HandleChangeSelectedSlot(FVector2D InputVal) override;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* ResumeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* RestartButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResumeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RestartText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuitText;

	/** 재시작 시 이동할 마을 맵 에셋 이름 (블루프린트 디폴트에서 실제 맵 이름으로 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Map")
	FName VillageLevelName = FName("VillageMap");

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	FButtonVisualSettings ButtonSettings;
	
	/** 키보드 선택 인덱스 변경 시 블루프린트에서 버튼 하이라이트/색상 연출 처리용 (0: 재개, 1: 재시작, 2: 종료) */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnSelectedButtonChanged(int32 NewIndex);

private:
	FOnMenuClosedSignature OnMenuClosedDelegate;
	int32 CurrentSelectedIndex = 0;

	UPROPERTY()
	TArray<UButton*> ActiveButtons;

	UPROPERTY()
	TArray<UTextBlock*> ActiveTextBlocks;

	void SetupActiveButtons();
	void RefreshButtonVisuals();
	void UpdateSelection(int32 NewIndex);

	UFUNCTION()
	void OnBtnHovered();

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnQuitClicked();
};
