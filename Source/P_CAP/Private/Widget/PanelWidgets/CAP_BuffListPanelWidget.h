// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Data/CAP_UIBuffListTypes.h"
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
	void AddOrUpdateBuffSlot(const FBuffSlotID& SlotID, const FBuffUIData& UIData);
	void RemoveBuffSlot(const FBuffSlotID& SlotID);
	class UCAP_ItemEffectSlot* FindSlot(const FBuffSlotID& SlotID) const;

	void OnGEAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	void OnGERemoved(const FActiveGameplayEffect& EffectRemoved);

	// 효과 발동시 호출될 함수
	UFUNCTION()
	void OnEffectTriggered(UObject* SourceObj, FGameplayTag DynamicTag, float Cooldown, float Duration, int32 Stacks);

	// 아이템 해제 감지용
	UFUNCTION()
	void HandleInventoryChanged(class UCAP_ItemInstance* ChangedItem, bool bIsAdded);
};
