// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Map/Widget/CombatRewardChoiceTypes.h"
#include "CombatRewardChoiceWidget.generated.h"

class ANextRoomChoiceManager;

UCLASS(BlueprintType, Blueprintable)
class UCombatRewardChoiceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Reward Choice")
	void InitializeChoiceWidget(ANextRoomChoiceManager* InChoiceManager, const TArray<FCombatRewardChoiceOption>& InOptions);

	UFUNCTION(BlueprintCallable, Category="Reward Choice")
	void ChooseReward(ECombatRoomRewardType RewardType);

	UFUNCTION(BlueprintCallable, Category="Reward Choice")
	void CloseChoiceWidget();

	UFUNCTION(BlueprintPure, Category="Reward Choice")
	TArray<FCombatRewardChoiceOption> GetOptions() const { return Options; }

	UFUNCTION(BlueprintPure, Category="Reward Choice")
	ANextRoomChoiceManager* GetChoiceManager() const { return ChoiceManager; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Reward Choice")
	void OnChoiceWidgetInitialized(const TArray<FCombatRewardChoiceOption>& InOptions);

private:
	UPROPERTY(BlueprintReadOnly, Category="Reward Choice", meta=(AllowPrivateAccess="true"))
	TObjectPtr<ANextRoomChoiceManager> ChoiceManager;

	UPROPERTY(BlueprintReadOnly, Category="Reward Choice", meta=(AllowPrivateAccess="true"))
	TArray<FCombatRewardChoiceOption> Options;
};
