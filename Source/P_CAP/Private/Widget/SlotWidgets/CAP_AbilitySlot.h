// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "CAP_AbilitySlot.generated.h"

/**
 * 스킬의 아이콘, 쿨타임 나타내는 슬롯 -> AbilityListView를 통해 리스트 형식으로 사용
 */
UCLASS()
class UCAP_AbilitySlot : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownDurationText;
};
