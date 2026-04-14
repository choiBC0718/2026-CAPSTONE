// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/CAP_MenuInterface.h"
#include "CAP_CharacterMenuWidget.generated.h"


/**
 * Tab키 입력으로 띄울 메인 위젯
 * GameplayWidget에서 스폰, WidgetSwitcher로 두가지 위젯 변경
 * (시너지 정보, 장착한 아이템, 아이템 정보 Tab) - InventoryTabWidget
 * (캐릭터 Attribute Tab) - AttributeTabWidget
 */
UCLASS()
class UCAP_CharacterMenuWidget : public UUserWidget, public ICAP_MenuInterface
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

	virtual void NativeOpenMenu() override;
	virtual void NativeCloseMenu() override;
	virtual FOnMenuClosedSignature& GetOnMenuClosedDelegate() override;
	
	void NavigationInput(FVector2D InputVal);
	void RefreshMenu();
	void SwitchCharacterMenuTab();

	UPROPERTY(BlueprintAssignable)
	FOnMenuClosedSignature OnMenuClosed;
	
private:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* SlideAnim;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* CloseAnim;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* SwitchTab;

	UPROPERTY(meta = (BindWidget))
	class UBorder* InventoryBorder;
	UPROPERTY(meta = (BindWidget))
	class UCAP_InventoryTabWidget* InventoryTabWidget;
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeTabWidget* AttributeTabWidget;

	bool bIsAttributeTabOpen = false;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
