// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "AI/PlayerBehaviorLearner.h"
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

	UFUNCTION()
	TMap<FName, int32> GetSavedStatEnhancedLevels() const;
	UFUNCTION()
	void UpdateSavedStatEnhanceLevel(FName RowName, int32 Level);

	UDataTable* GetKeyIconTable() const {return KeyIconTable;}
	UDataTable*	GetStatEnhanceTable() const {return StatEnhanceDataTable;}

	// Assessment 성향 — StageGoalTrigger(assessment mode)가 저장, MapManager가 읽음
	void SetAssessmentTendency(const FPlayerTendencyModifier& Tendency);
	const FPlayerTendencyModifier& GetAssessmentTendency() const { return AssessmentTendency; }
	bool HasAssessmentTendency() const { return bHasAssessmentTendency; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "GlobalData")
	class UDataTable* KeyIconTable;
	UPROPERTY(EditDefaultsOnly, Category = "GlobalData")
	class UDataTable* StatEnhanceDataTable;
	
private:
	UPROPERTY()
	class UCAP_SaveGame* CurrentSaveGame;

	FString SaveSlotName = TEXT("MainSaveSlot");
	uint32 UserIndex=0;

	FPlayerTendencyModifier AssessmentTendency;
	bool bHasAssessmentTendency = false;
};
