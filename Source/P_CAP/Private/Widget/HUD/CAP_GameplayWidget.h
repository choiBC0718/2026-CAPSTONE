// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_CharacterMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "Interactables/NPC/CAP_WorldNPC.h"
#include "Widget/PanelWidgets/CAP_ItemSwapWidget.h"
#include "Widget/PanelWidgets/CAP_PickupDetailPanelWidget.h"
#include "CAP_GameplayWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_GameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	FORCEINLINE UCAP_CharacterMenuWidget* GetCharacterMenuWidget() const {return CharacterMenuWidget;}
	FORCEINLINE UCAP_ItemSwapWidget* GetItemSwapWidget() const { return ItemSwapWidget; }

	// PlayerController에서 바인딩 되는 메소드
	void ToggleCharacterMenu();
	void HideMenu();
	void UINavigationHandle(FVector2D InputVal);	// UI 내부 WASD로 슬롯 포커스 변경
	void RouteUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime);
	void ShowStatisticDashboard();

protected:
	UPROPERTY(meta = (BindWidget))
	class UCAP_WeaponSwapWidget* WeaponSwapWidget;
	// Hp 바
	UPROPERTY(meta = (BindWidget))
	class UCAP_ValueGauge* HealthBar;
	// 장착한 무기에 부여된 스킬 아이콘
	UPROPERTY(meta = (BindWidget))
	class UCAP_WeaponSkillBox* WeaponAbilityPanelWidget;
	// 상호작용 대상 오버랩 시 보여줄 패널
	UPROPERTY(meta = (BindWidget))
	class UCAP_PickupDetailPanelWidget* InteractPanelWidget;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;
	// Tab 키로 띄울 위젯
	UPROPERTY(meta = (BindWidget))
	class UCAP_CharacterMenuWidget* CharacterMenuWidget;
	// 인벤토리 꽉 찬 경우 띄울 아이템 교체 위젯
	UPROPERTY(meta = (BindWidget))
	class UCAP_ItemSwapWidget* ItemSwapWidget;
	// 게임 종료 시 나타낼 대시보드
	UPROPERTY(meta = (BindWidget))
	class UCAP_StatisticDashboardWidget* StatDashboardWidget;

	// 아이템 버프 리스트 위젯
	UPROPERTY(meta = (BindWidget))
	class UCAP_BuffListPanelWidget* BuffListPanel;
	
	UPROPERTY(meta = (BindWidget))
	class UCAP_CurrencyPanelWidget* CurrencyPanel;
	
	UPROPERTY(meta = (BindWidget))
	class UCAP_DialogueWidget* DialogueWidget;
	
private:
	UPROPERTY()
	ACAP_PlayerCharacter* Player;
	UPROPERTY()
	UUserWidget* CurrentActiveMenu;

	// 인벤토리 꽉 찼을 때 변경을 위한 위젯 활성화
	UFUNCTION()
	void HandleInventoryFull(class UCAP_ItemInstance* NewItem);
	// 대화 종료 시 콜리전 겹쳐있는 액터가 있으면 위젯 표출하도록
	UFUNCTION()
	void HandleDialogueFinished();
	UFUNCTION()
	void HandleNPCCustomWidgetOpen(UUserWidget* TargetWidget);
	// 
	UFUNCTION()
	void OnActiveMenuClosed();
	
	bool CanProcessUINavigation();

	void ShowMenu(UUserWidget* TargetMenuWidget);
	void SetGamePause(bool Pause);

};
