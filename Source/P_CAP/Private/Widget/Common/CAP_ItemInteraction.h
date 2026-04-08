// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
 * 아이템과 상호작용 가능할 때 나타날 위젯
 */
UCLASS()
class UCAP_ItemInteraction : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetInteractionUIVisibility(bool bVisible);
	void UpdateInteractProgress(float Progress);
	void SetInteractKeyText(const FString& KeyName);
	
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* EquipIconImg;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EquipText;
	
	UPROPERTY(meta = (BindWidget))
	class UImage* DisassembleIconImg;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisassembleText;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar;
	UPROPERTY(meta = (BindWidget))
	class UImage* InteractProgressImage;

	UPROPERTY()
	class UMaterialInstanceDynamic* ProgressMID;
	
	UPROPERTY(EditDefaultsOnly, Category="Data")
	class UDataTable* KeyIconDataTable;
};
