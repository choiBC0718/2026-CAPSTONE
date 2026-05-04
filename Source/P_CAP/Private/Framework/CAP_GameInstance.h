// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION()
	void SaveGameData();
	UFUNCTION()
	void LoadGameData();
	UFUNCTION()
	int32 GetSavedMagicStone() const;
	UFUNCTION()
	void OnCurrencyChanged(ECurrencyType Type, int32 OldAmount, int32 NewAmount);

private:
	UPROPERTY()
	class UCAP_SaveGame* CurrentSaveGame;

	FString SaveSlotName = TEXT("MainSaveSlot");
	uint32 UserIndex=0;
};
