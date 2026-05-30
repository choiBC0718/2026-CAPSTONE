// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Map/Widget/CombatRewardChoiceTypes.h"
#include "CombatRewardChoiceCardWidget.generated.h"

class UImage;
class UBorder;
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
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintImplementableEvent, Category="Reward Choice")
	void OnCardInitialized(const FCombatRewardChoiceOption& InOption);

private:
	UPROPERTY(EditAnywhere, Category="Reward Choice|Style", meta=(AllowPrivateAccess="true"))
	FLinearColor GoldAccentColor = FLinearColor(1.0f, 0.68f, 0.18f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Reward Choice|Style", meta=(AllowPrivateAccess="true"))
	FLinearColor ItemAccentColor = FLinearColor(0.18f, 0.78f, 0.48f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Reward Choice|Style", meta=(AllowPrivateAccess="true"))
	FLinearColor WeaponAccentColor = FLinearColor(0.75f, 0.28f, 0.18f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Reward Choice|Style", meta=(AllowPrivateAccess="true"))
	FLinearColor DefaultAccentColor = FLinearColor(0.72f, 0.72f, 0.72f, 1.0f);

	UPROPERTY(EditAnywhere, Category="Reward Choice|Style", meta=(AllowPrivateAccess="true"))
	FLinearColor CardNormalColor = FLinearColor(0.055f, 0.052f, 0.06f, 0.94f);

	UPROPERTY(EditAnywhere, Category="Reward Choice|Style", meta=(AllowPrivateAccess="true"))
	FLinearColor CardHoveredColor = FLinearColor(0.105f, 0.095f, 0.09f, 0.98f);

	UPROPERTY(EditAnywhere, Category="Reward Choice|Style", meta=(AllowPrivateAccess="true"))
	float HoverScale = 1.04f;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Title;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Description;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Badge;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Cost;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_TypeGlyph;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> Image_Artwork;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UBorder> Border_CardFrame;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UBorder> Border_Accent;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UBorder> Border_ArtworkFrame;

	UPROPERTY(BlueprintReadOnly, Category="Reward Choice", meta=(AllowPrivateAccess="true"))
	FCombatRewardChoiceOption Option;

	void RefreshCard();
	void ApplyVisualState(bool bHovered);
	FLinearColor GetAccentColor() const;
	FText GetDefaultTitle() const;
	FText GetDefaultDescription() const;
	FText GetDefaultBadgeText() const;
	FText GetTypeGlyph() const;
};
