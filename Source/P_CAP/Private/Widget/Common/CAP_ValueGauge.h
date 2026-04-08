// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "CAP_ValueGauge.generated.h"

/**
 * HP 나타낼 바 위젯
 */
UCLASS()
class UCAP_ValueGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetAndBoundToGameplayAttribute(class UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute);
	void SetValue(float NewValue, float NewMaxValue);

private:
	void ValueChanged(const FOnAttributeChangeData& ChangedData);
	void MaxValueChanged(const FOnAttributeChangeData& ChangedData);

	float CachedValue;
	float CachedMaxValue;

	UPROPERTY(EditAnywhere, Category="Visual")
	FLinearColor BarColor;
	UPROPERTY(EditAnywhere, Category="Visual")
	FSlateFontInfo ValueTextFont;
	UPROPERTY(EditAnywhere, Category="Visual")
	bool bValueTextVisible = true;
	UPROPERTY(EditAnywhere, Category="Visual")
	bool bProgressBarVisible = true;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	class UProgressBar* ProgressBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	class UTextBlock* ValueText;
};
