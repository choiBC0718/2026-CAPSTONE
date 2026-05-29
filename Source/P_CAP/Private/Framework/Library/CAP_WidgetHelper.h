// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CAP_WidgetHelper.generated.h"

// 버튼 스타일 데이터 패키지
USTRUCT(BlueprintType)
struct FButtonVisualSettings
{
	GENERATED_BODY()
	// 노말 배경 색
	UPROPERTY(EditAnywhere, Category="Visuals|Normal")
	FLinearColor NormalBgColor = FLinearColor::White;
	// 노말 외곽 색
	UPROPERTY(EditAnywhere, Category="Visuals|Normal")
	FLinearColor NormalOutlineColor = FLinearColor::Transparent;
	// 노말 외곽 두께
	UPROPERTY(EditAnywhere, Category="Visuals|Normal")
	float NormalOutlineWidth = 3.f;
	// 노말 내부 폰트 사이즈
	UPROPERTY(EditAnywhere, Category="Visuals|Normal")
	int32 NormalFontSize = 30;

	// 호버 배경 색
	UPROPERTY(EditAnywhere, Category="Visuals|Hover")
	FLinearColor HoverBgColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.f);
	// 호버 외곽 색
	UPROPERTY(EditAnywhere, Category="Visuals|Hover")
	FLinearColor HoverOutlineColor = FLinearColor::White;
	// 호버 외각 두께
	UPROPERTY(EditAnywhere, Category="Visuals|Hover")
	float HoverOutlineWidth = 3.f;
	// 호버 내부 폰트 사이즈
	UPROPERTY(EditAnywhere, Category="Visuals|Hover")
	int32 HoverFontSize = 36;

	// 눌림 배경색
	UPROPERTY(EditAnywhere, Category="Visuals|Pressed")
	FLinearColor PressedBgColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	// 눌림 외곽 색
	UPROPERTY(EditAnywhere, Category="Visuals|Pressed")
	FLinearColor PressedOutlineColor = FLinearColor::White;
	// 눌림 외곽 두께
	UPROPERTY(EditAnywhere, Category="Visuals|Pressed")
	float PressedOutlineWidth = 3.f;
};
/**
 * 
 */
UCLASS()
class UCAP_WidgetHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION()
	static void ApplyCustomButtonStyle(class UButton* Button, class UTextBlock* TextBlock, bool bIsFocused, const FButtonVisualSettings& Settings);
};
