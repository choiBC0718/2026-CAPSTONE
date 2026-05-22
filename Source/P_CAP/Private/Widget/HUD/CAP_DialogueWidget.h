// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Interactables/NPC/CAP_WorldNPC.h"
#include "CAP_DialogueWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNPCCustomWidgetOpened, UUserWidget*, TargetWidget);
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
	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent) override;
	
	UPROPERTY()
	FOnDialogueFinished OnDialogueFinished;
	UPROPERTY()
	FOnNPCCustomWidgetOpened OnNPCCustomWidgetOpened;
	
	void SetButtonVisualFocus(class UButton* FocusedBtn);
	void ClearButtonVisualFocus();
	
protected:
	// 대화 시작 : 대화 애니메이션 + 입력 금지
	void StartDialogue();
	// 버튼과 NPCData를 통한 여러 텍스트 초기화
	void UpdateDialogueUI(const struct FNPCData& Data);

	UFUNCTION()
	void OnSpecialBtnClicked();
	UFUNCTION()
	void OnTalkBtnClicked();
	UFUNCTION()
	void OnQuitBtnClicked();
	
	UFUNCTION()
	void OnBtnHovered();

	UFUNCTION()
	void OnNPCDialogueStarted(const FNPCData& NPCData);
	
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
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	class UWidgetAnimation* ChangeToCustomView;

	// 특정 위젯 바인딩 X, 빈 상자를 배치함
	UPROPERTY(meta=(BindWidget))
	class UNamedSlot* CustomMenuWidget;
	// 빈 상자에 들어갈 위젯을 기억할 포인터
	UPROPERTY()
	class UUserWidget* ActiveCustomWidget;

	bool bIsClosing = false;
	bool bIsCustomWidgetClosing = false;
	// 현재 포커스 되있는 버튼 인덱스
	int32 CurrentSelectedIndex = 0;

	UPROPERTY()
	TArray<class UTextBlock*> ActiveTextBlocks;
	UPROPERTY()
	TArray<class UButton*> ActiveButtons;
	

	// 포커스 상태의 버튼 색
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FLinearColor ButtonHoverColor = FLinearColor::White;
	// 일반 상태의 버튼 색
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FLinearColor ButtonNormalColor = FLinearColor::White;
	// 포커스 상태의 버튼 경계선 색
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	FLinearColor ButtonHoverOutlineColor = FLinearColor::White;
	// 포커스 상태의 버튼 경계선 두께
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	float ButtonHoverOutlineWidth = 4.f;

	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	int32 NormalFontSize = 20;
	UPROPERTY(EditDefaultsOnly, Category="Visuals")
	int32 HoverFontSize = 26;

	UPROPERTY()
	class ACAP_PlayerCharacter* Player;
	
	UPROPERTY()
	FNPCData CachedNPCData;

	void OpenCustomWidget();
	UFUNCTION()
	void CloseCustomWidget();
	void HandleFirstInteraction(ENPCActionResult Result);
	void HandleRequireConfirm(ENPCActionResult Result);
	void HandleFailedInteraction(ENPCActionResult Result);
	
	void RefreshButtonVisuals();
	// SpecialBtn 선택 시 대화 버튼 없앰, '수락' '거절' 텍스트로 수정
	void ChangeToConfirmState(const FString& ConfirmText);
	// SpecialBtn 선택 시 종료를 위한 상태로 만들기
	void ChangeToRewardState(const FString& ResultText);

	void SetupActiveButtons(bool bShowTalk, bool bShowSpecial,bool bShowQuit);
};
