// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_CharacterMenuWidget.h"
#include "Blueprint/UserWidget.h"
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
	
	bool IsCharacterMenuOpen();		// 인벤토리 메뉴 열려있는지 확인
	bool IsItemSwapMenuOpen();		// 아이템 변경 메뉴 열려있는지 확인
	
	void ActivateSwitcher();		// 인벤토리 메뉴 열기 (MenuSwitch 켜기)
	void DeactivateSwitcher();		// MenuSwitch 끄기
	void SwitchCharacterMenuTab();	
	void OpenItemSwapMenu(class UCAP_ItemInstance* NewItem);	// 인벤토리 꽉 찼을 때 변경을 위한 위젯 활성화

	void UpdateInteractProgress(float Progress);				// 키 누르는 동안 게이지 증가
	void UpdateInteractionUI(bool bVisible, UObject* ItemData, const FString& KeyName);	//상호작용 할 아이템으로 UI 업데이트
	
protected:
	// Hp 바
	UPROPERTY(meta = (BindWidget))
	class UCAP_ValueGauge* HealthBar;
	// 부여된 스킬 아이콘 List View
	UPROPERTY(meta = (BindWidget))
	class UCAP_AbilityListView* AbilityListView;
	// 아이템 상호작용 시, 아이템의 디테일 값 나타낼 위젯
	UPROPERTY(meta = (BindWidget))
	class UCAP_PickupDetailPanelWidget* PickupItemDetailWidget;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;
	// Tab 키로 띄울 위젯
	UPROPERTY(meta = (BindWidget))
	class UCAP_CharacterMenuWidget* CharacterMenuWidget;
	UPROPERTY(meta = (BindWidget))
	class UCAP_ItemSwapWidget* ItemSwapWidget;
	
private:
	UPROPERTY()
	class UAbilitySystemComponent* OwnerASC;

	
	UFUNCTION()
	void HandleWeaponChanged(class UCAP_WeaponInstance* NewWeaponInstance);
};
