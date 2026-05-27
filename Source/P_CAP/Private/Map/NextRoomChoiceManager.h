// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/RoomActor/DoorDirection.h"
#include "Map/RoomTypes.h"
#include "Map/Widget/CombatRewardChoiceTypes.h"
#include "NextRoomChoiceManager.generated.h"

class ACharacter;
class AMapManager;
class UDataTable;
class UCombatRewardChoiceWidget;
struct FRoomData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatRewardChoiceRequested, FIntPoint, TargetRoomPos);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatRewardChoiceCompleted, FIntPoint, TargetRoomPos, ECombatRoomRewardType, SelectedRewardType);

UCLASS()
class ANextRoomChoiceManager : public AActor
{
	GENERATED_BODY()

public:
	ANextRoomChoiceManager();

	UPROPERTY(BlueprintAssignable, Category="Room Choice")
	FOnCombatRewardChoiceRequested OnCombatRewardChoiceRequested;

	UPROPERTY(BlueprintAssignable, Category="Room Choice")
	FOnCombatRewardChoiceCompleted OnCombatRewardChoiceCompleted;

	void RequestEnterRoom(ACharacter* PlayerCharacter, const FIntPoint& TargetRoomPos, EDoorDirection ExitDirection);

	UFUNCTION(BlueprintCallable, Category="Room Choice")
	void SelectGoldReward();

	UFUNCTION(BlueprintCallable, Category="Room Choice")
	void SelectItemReward();

	UFUNCTION(BlueprintCallable, Category="Room Choice")
	void SelectReward(ECombatRoomRewardType RewardType);

	UFUNCTION(BlueprintPure, Category="Room Choice")
	bool IsWaitingForCombatRewardChoice() const { return bWaitingForCombatRewardChoice; }

	UFUNCTION(BlueprintPure, Category="Room Choice")
	FIntPoint GetPendingTargetRoomPos() const { return PendingTargetRoomPos; }

	UFUNCTION(BlueprintPure, Category="Room Choice")
	TArray<FCombatRewardChoiceOption> GetCurrentChoiceOptions() const { return CurrentChoiceOptions; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category="Room Choice", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AMapManager> MapManager;

	UPROPERTY(EditAnywhere, Category="Room Choice|Data", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDataTable> CombatRewardChoiceTable;

	UPROPERTY(EditAnywhere, Category="Room Choice|Data", meta=(AllowPrivateAccess="true"))
	TArray<FName> DefaultChoiceRowNames;

	UPROPERTY(EditAnywhere, Category="Room Choice|UI", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UCombatRewardChoiceWidget> ChoiceWidgetClass;

	UPROPERTY()
	TObjectPtr<UCombatRewardChoiceWidget> ActiveChoiceWidget;

	UPROPERTY()
	TObjectPtr<ACharacter> PendingPlayerCharacter;

	UPROPERTY()
	TArray<FCombatRewardChoiceOption> CurrentChoiceOptions;

	FIntPoint PendingTargetRoomPos = FIntPoint::ZeroValue;
	EDoorDirection PendingExitDirection = EDoorDirection::Up;
	bool bWaitingForCombatRewardChoice = false;

	AMapManager* ResolveMapManager();
	void BindInput();
	bool DoesRoomNeedCombatRewardChoice(const FRoomData& RoomData) const;
	TArray<FCombatRewardChoiceOption> BuildCombatRewardChoiceOptions() const;
	void ShowChoiceWidget();
	void HideChoiceWidget();
	void BeginCombatRewardChoice(ACharacter* PlayerCharacter, const FIntPoint& TargetRoomPos, EDoorDirection ExitDirection);
	void ApplyCombatRewardChoice(ECombatRoomRewardType SelectedRewardType);
	void ClearPendingChoice();
};
