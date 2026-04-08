// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget/Common/CAP_ItemInteraction.h"
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
	
	bool IsCharacterMenuOpen();
	void OpenCharacterMenu();
	void CloseCharacterMenu();
	void SwitchCharacterMenuTab();
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UCAP_ValueGauge* HealthBar;
	UPROPERTY(meta = (BindWidget))
	class UCAP_ItemInteraction* InteractionWidget;
	// 부여된 스킬 아이콘 List View
	UPROPERTY(meta = (BindWidget))
	class UCAP_AbilityListView* AbilityListView;

	// Tab 키로 띄울 위젯
	UPROPERTY(meta = (BindWidget))
	class UCAP_CharacterMenuWidget* CharacterMenuWidget;
	
private:
	UPROPERTY()
	class UAbilitySystemComponent* OwnerASC;

	
	UFUNCTION()
	void HandleWeaponChanged(class UCAP_WeaponInstance* NewWeaponInstance);
};
