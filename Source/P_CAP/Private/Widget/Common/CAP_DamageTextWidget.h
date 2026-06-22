// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_DamageTextWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamageAnimFinished);
/**
 * 
 */
UCLASS()
class UCAP_DamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnDamageAnimFinished OnDamageAnimFinished;

	UFUNCTION()
	void PlayDamageAnimation(float DamageAmount, bool bIsCritical, bool bIsPlayer);

protected:
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DamageText;
	
	UPROPERTY(Transient, meta=(BindWidgetAnim))
	class UWidgetAnimation* NormalDamageAnim;
	UPROPERTY(Transient, meta=(BindWidgetAnim))
	class UWidgetAnimation* CriticalDamageAnim;
	UPROPERTY(Transient, meta=(BindWidgetAnim))
	class UWidgetAnimation* PlayerDamageAnim;
};
