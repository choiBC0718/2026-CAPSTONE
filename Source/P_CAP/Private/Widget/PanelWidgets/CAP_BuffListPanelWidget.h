// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "CAP_BuffListPanelWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_BuffListPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void virtual NativeConstruct() override;
	void InitializeWidget(class ACAP_PlayerCharacter* InPlayerCharacter);

protected:
	// 영구적인 꺼지지 않을 아이템 효과를 나타내는 WrapBox
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* PassiveWrapBox;
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* BuffWrapBox;
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UCAP_ItemEffectSlot> EffectSlotClass;

	UPROPERTY()
	TArray<class UCAP_ItemEffectSlot*> ActiveSlots;
private:

	// 효과 발동시 호출될 함수
	UFUNCTION()
	void OnEffectTriggered(class UCAP_ItemInstance* ItemInst, FGameplayTag DynamicTag, float Cooldown, float Duration, int32 Stacks);

	// 아이템 해제 감지용
	UFUNCTION()
	void HandleInventoryChanged(class UCAP_ItemInstance* ChangedItem, bool bIsAdded);

	// 중복 슬롯 검색
	class UCAP_ItemEffectSlot* FindExistingSlot(class UCAP_ItemInstance* ItemInst, FGameplayTag DynamicTag) const;

	void CleanUpInvalidSlots();
};
