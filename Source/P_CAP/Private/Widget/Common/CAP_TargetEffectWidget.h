// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "CAP_TargetEffectWidget.generated.h"

struct FTargetStackInfo
{
	int32 CurrentStack=0;
	int32 MaxStack=0;
};
/**
 * 
 */
UCLASS()
class UCAP_TargetEffectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void UpdateEffectUI(const FGameplayTag& BehaviorTag,int32 CurrentStack, int32 MaxStack);

protected:
	UPROPERTY(meta = (BindWidget))
	class USizeBox* StackSizeBox;
	UPROPERTY(meta = (BindWidget))
	class UImage* StackBarImage;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	float WidthPerStack = 40.f;
	UPROPERTY()
	class UMaterialInstanceDynamic* StackMID;
private:
	TMap<FGameplayTag, FTargetStackInfo> ActiveStacks;
};
