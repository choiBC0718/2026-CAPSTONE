// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "CAP_SynergySlotWidget.generated.h"

/**
 * InventoryTabWidget에 들어갈 패널 (소유한 아이템 시너지)의 요소
 */
UCLASS()
class UCAP_SynergySlotWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* SynergyNameText;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CurrentLevelText;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* LevelRequireText;
};
