// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/RoomActor/DoorDirection.h"
#include "Map/RoomTypes.h"
#include "NextRoomChoiceManager.generated.h"

class ACharacter;
class AMapManager;
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

	UFUNCTION(BlueprintPure, Category="Room Choice")
	bool IsWaitingForCombatRewardChoice() const { return bWaitingForCombatRewardChoice; }

	UFUNCTION(BlueprintPure, Category="Room Choice")
	FIntPoint GetPendingTargetRoomPos() const { return PendingTargetRoomPos; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category="Room Choice", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AMapManager> MapManager;

	UPROPERTY()
	TObjectPtr<ACharacter> PendingPlayerCharacter;

	FIntPoint PendingTargetRoomPos = FIntPoint::ZeroValue;
	EDoorDirection PendingExitDirection = EDoorDirection::Up;
	bool bWaitingForCombatRewardChoice = false;

	AMapManager* ResolveMapManager();
	void BindInput();
	bool DoesRoomNeedCombatRewardChoice(const FRoomData& RoomData) const;
	void BeginCombatRewardChoice(ACharacter* PlayerCharacter, const FIntPoint& TargetRoomPos, EDoorDirection ExitDirection);
	void ApplyCombatRewardChoice(ECombatRoomRewardType SelectedRewardType);
	void ClearPendingChoice();
};
