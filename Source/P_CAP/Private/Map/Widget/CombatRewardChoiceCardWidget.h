// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Map/Widget/CombatRewardChoiceTypes.h"
#include "CombatRewardChoiceCardWidget.generated.h"

class UImage;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatRewardChoiceCardSelected, ECombatRoomRewardType, RewardType);

UCLASS(BlueprintType, Blueprintable)
class UCombatRewardChoiceCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Reward Choice")
	FOnCombatRewardChoiceCardSelected OnCardSelected;

	UFUNCTION(BlueprintCallable, Category="Reward Choice")
	void InitializeCard(const FCombatRewardChoiceOption& InOption);

	UFUNCTION(BlueprintCallable, Category="Reward Choice")
	void SelectCard();

	UFUNCTION(BlueprintPure, Category="Reward Choice")
	FCombatRewardChoiceOption GetOption() const { return Option; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Reward Choice")
	void OnCardInitialized(const FCombatRewardChoiceOption& InOption);

private:
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Title;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Description;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Badge;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Cost;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_Artwork;

	UPROPERTY(BlueprintReadOnly, Category="Reward Choice", meta=(AllowPrivateAccess="true"))
	FCombatRewardChoiceOption Option;

	void RefreshCard();
};
