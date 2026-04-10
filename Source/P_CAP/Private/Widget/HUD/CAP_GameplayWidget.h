// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_CharacterMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "Widget/Common/CAP_ItemInteraction.h"
#include "Widget/Item/CAP_ItemSwapWidget.h"
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

	FORCEINLINE UCAP_ItemInteraction* GetInteractionWidget() const {return InteractionWidget;}
	FORCEINLINE UCAP_CharacterMenuWidget* GetCharacterMenuWidget() const {return CharacterMenuWidget;}
	FORCEINLINE UCAP_ItemSwapWidget* GetItemSwapWidget() const { return ItemSwapWidget; }
	
	bool IsCharacterMenuOpen();		// 인벤토리 메뉴 열려있는지 확인
	bool IsItemSwapMenuOpen();		// 아이템 변경 메뉴 열려있는지 확인
	
	void ActivateSwitcher();		// 인벤토리 메뉴 열기 (MenuSwitch 켜기)
	void DeactivateSwitcher();		// MenuSwitch 끄기
	void SwitchCharacterMenuTab();	
	void OpenItemSwapMenu(class UCAP_ItemInstance* NewItem);
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UCAP_ValueGauge* HealthBar;
	UPROPERTY(meta = (BindWidget))
	class UCAP_ItemInteraction* InteractionWidget;
	// 부여된 스킬 아이콘 List View
	UPROPERTY(meta = (BindWidget))
	class UCAP_AbilityListView* AbilityListView;

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
