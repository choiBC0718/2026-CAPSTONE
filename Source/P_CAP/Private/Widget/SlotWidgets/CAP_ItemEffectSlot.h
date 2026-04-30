// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "CAP_ItemEffectSlot.generated.h"

/**
 * 아이템 효과의 쿨타임, 지속시간, 스택을 나타내는 위젯
 */
UCLASS()
class UCAP_ItemEffectSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void InitSlot(class UCAP_ItemInstance* InItemInst, FGameplayTag InDynamicTag, float InCooldown, float InDuration, int32 InStacks);

	class UCAP_ItemInstance* GetItemInstance() const {return ItemInst;}
	FGameplayTag GetDynamicTag() const {return DynamicTag;}
private:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	float UpdateInterval = 0.05f;
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName IconMaterialParamName = "Icon";
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName CooldownPercentParamName = "CooldownPercent";
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName DurationPercentParamName = "DurationPercent";

	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* StackText;

	UPROPERTY()
	class UCAP_ItemInstance* ItemInst;

	FGameplayTag DynamicTag;
	float MaxCooldown;
	float MaxDuration;
	float RemainingCooldown;
	float RemainingDuration;
	int32 CurrentStack;

	FTimerHandle UpdateTimerHandle;

	void StartTimers();
	void UpdateGauges();
};
