// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "CAP_DialogueWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueFinished);
/**
 * 
 */
UCLASS()
class UCAP_DialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
	// UIOnly일때 키보드 입력 가로채는 함수
	virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent) override;
	
	void StartDialogue();
	void UpdateDialogueUI(const struct FNPCData& Data);
	void ChangeToRewardState(const FString& ResultText);

	UPROPERTY()
	FOnDialogueFinished OnDialogueFinished;

protected:
	UFUNCTION()
	void OnSpecialBtnClicked();
	UFUNCTION()
	void OnTalkBtnClicked();
	UFUNCTION()
	void OnQuitBtnClicked();
	
	UFUNCTION()
	void OnBtnHovered();
	
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* NPC_Image;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NPC_Name;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DialogueText;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SpecialActionText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Talk;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Quit;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* SpecialBtn;
	UPROPERTY(meta = (BindWidget))
	class UButton* TalkBtn;
	UPROPERTY(meta = (BindWidget))
	class UButton* QuitBtn;
	
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	class UWidgetAnimation* StartDialogueAnim;

	bool bIsClosing = false;
	int32 CurrentSelectedIndex = 0;

	UPROPERTY()
	TArray<class UTextBlock*> ActiveTextBlocks;
	UPROPERTY()
	TArray<class UButton*> ActiveButtons;
	
	void RefreshButtonVisuals();

	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FLinearColor ButtonHoverColor = FLinearColor::White;
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FLinearColor ButtonNormalColor = FLinearColor::White;
	
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FLinearColor ButtonHoverOutlineColor = FLinearColor::White;
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	float ButtonHoverOutlineWidth = 4.f;

	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	int32 NormalFontSize = 20;
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	int32 HoverFontSize = 26;

	UPROPERTY()
	ACAP_PlayerCharacter* Player;
};
