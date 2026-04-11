// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "CAP_AttributeSlotWidget.generated.h"

/**
 * 1개의 Attribute를 나타낼 위젯
 * 여러 스탯들을 AttributeTabWidget에 넣음 
 */
UCLASS()
class UCAP_AttributeSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
private:
	// 스탯 이름
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AttributeNameText;
	// 스탯 값
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AttributeValueText;
	// 스탯 이름에 맞는 Attribute
	UPROPERTY(EditAnywhere, Category="Attribute")
	FGameplayAttribute Attribute;
	UPROPERTY(EditAnywhere, Category="Attribute")
	FText AttributeName;

	UPROPERTY()
	class UAbilitySystemComponent* OwnerASC;

	void UpdatePercentage();
	void AttributeChanged(const FOnAttributeChangeData& Data);

	FNumberFormattingOptions NumberFormattingOptions;
};
