// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interface/CAP_InteractInterface.h"
#include "CAP_ItemInteraction.generated.h"

USTRUCT(BlueprintType)
struct FKeyIconRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* Icon = nullptr;
};

/**
 * 상호작용 대상과 오버랩 시 나타날 텍스트 + 상호작용 키 이미지 위젯
 */
UCLASS()
class UCAP_ItemInteraction : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetInteractionUIVisibility(bool bVisible);
	void SetInteractKeyText(const FString& KeyName);

	void UpdateActionTexts(const FInteractionPayload& Payload);
private:
	UPROPERTY(meta = (BindWidget))
	class UBorder* TapInteractBorder;
	UPROPERTY(meta = (BindWidget))
	class UImage* EquipIconImg;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EquipText;
	
	UPROPERTY(meta = (BindWidget))
	class UBorder* HoldInteractBorder;
	UPROPERTY(meta = (BindWidget))
	class UImage* DisassembleIconImg;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisassembleText;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar;
	UPROPERTY(meta = (BindWidget))
	class UImage* InteractProgressImage;
	UPROPERTY(meta = (BindWidget))
	class UImage* DisassembleRewardIcon;

	UPROPERTY()
	class UMaterialInstanceDynamic* ProgressMID;
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TMap<ECurrencyType, class UTexture2D*> CurrencyIconMap;

	UFUNCTION()
	void UpdateInteractProgress(float Progress);
};
