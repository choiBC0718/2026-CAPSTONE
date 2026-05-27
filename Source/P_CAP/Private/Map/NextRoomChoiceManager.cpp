// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/NextRoomChoiceManager.h"

#include "Engine/Engine.h"
#include "Components/InputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Map/RoomActor/RoomActor.h"
#include "Kismet/GameplayStatics.h"
#include "Map/Debug/MapManager.h"
#include "Map/RoomData.h"

ANextRoomChoiceManager::ANextRoomChoiceManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ANextRoomChoiceManager::BeginPlay()
{
	Super::BeginPlay();

	ResolveMapManager();
	BindInput();
}

void ANextRoomChoiceManager::RequestEnterRoom(
	ACharacter* PlayerCharacter,
	const FIntPoint& TargetRoomPos,
	EDoorDirection ExitDirection)
{
	if (!PlayerCharacter)
	{
		return;
	}

	AMapManager* ResolvedMapManager = ResolveMapManager();
	if (!ResolvedMapManager)
	{
		return;
	}

	const FRoomData* TargetRoomData = ResolvedMapManager->FindRoomData(TargetRoomPos);
	if (!TargetRoomData)
	{
		return;
	}

	if (DoesRoomNeedCombatRewardChoice(*TargetRoomData))
	{
		BeginCombatRewardChoice(PlayerCharacter, TargetRoomPos, ExitDirection);
		return;
	}

	ResolvedMapManager->MovePlayerToRoom(PlayerCharacter, TargetRoomPos, ExitDirection);
}

void ANextRoomChoiceManager::SelectGoldReward()
{
	ApplyCombatRewardChoice(ECombatRoomRewardType::Gold);
}

void ANextRoomChoiceManager::SelectItemReward()
{
	ApplyCombatRewardChoice(ECombatRoomRewardType::Item);
}

AMapManager* ANextRoomChoiceManager::ResolveMapManager()
{
	if (MapManager)
	{
		return MapManager;
	}

	MapManager = Cast<AMapManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapManager::StaticClass()));
	return MapManager;
}

void ANextRoomChoiceManager::BindInput()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	EnableInput(PC);

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ANextRoomChoiceManager::SelectGoldReward);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ANextRoomChoiceManager::SelectItemReward);
	}
}

bool ANextRoomChoiceManager::DoesRoomNeedCombatRewardChoice(const FRoomData& RoomData) const
{
	return RoomData.RoomType == ERoomType::Normal &&
		RoomData.CombatRewardType == ECombatRoomRewardType::None;
}

void ANextRoomChoiceManager::BeginCombatRewardChoice(
	ACharacter* PlayerCharacter,
	const FIntPoint& TargetRoomPos,
	EDoorDirection ExitDirection)
{
	if (bWaitingForCombatRewardChoice)
	{
		return;
	}

	PendingPlayerCharacter = PlayerCharacter;
	PendingTargetRoomPos = TargetRoomPos;
	PendingExitDirection = ExitDirection;
	bWaitingForCombatRewardChoice = true;
	OnCombatRewardChoiceRequested.Broadcast(TargetRoomPos);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("NextRoomChoiceManager: combat reward choice required for room (%d, %d)"),
		TargetRoomPos.X,
		TargetRoomPos.Y);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			4.0f,
			FColor::Cyan,
			TEXT("Choose combat reward: 1 Gold / 2 Item"));
	}
}

void ANextRoomChoiceManager::ApplyCombatRewardChoice(ECombatRoomRewardType SelectedRewardType)
{
	if (!bWaitingForCombatRewardChoice)
	{
		return;
	}

	AMapManager* ResolvedMapManager = ResolveMapManager();
	if (!ResolvedMapManager || !PendingPlayerCharacter)
	{
		ClearPendingChoice();
		return;
	}

	FRoomData* TargetRoomData = ResolvedMapManager->FindRoomData(PendingTargetRoomPos);
	if (!TargetRoomData || !DoesRoomNeedCombatRewardChoice(*TargetRoomData))
	{
		ClearPendingChoice();
		return;
	}

	TargetRoomData->CombatRewardType = SelectedRewardType;
	OnCombatRewardChoiceCompleted.Broadcast(PendingTargetRoomPos, SelectedRewardType);

	if (ARoomActor* TargetRoomActor = ResolvedMapManager->FindSpawnedRoomByGridPos(PendingTargetRoomPos))
	{
		TargetRoomActor->SetCombatRewardType(SelectedRewardType);
	}

	const FIntPoint TargetRoomPos = PendingTargetRoomPos;
	const EDoorDirection ExitDirection = PendingExitDirection;
	ACharacter* PlayerCharacter = PendingPlayerCharacter;

	ClearPendingChoice();

	ResolvedMapManager->MovePlayerToRoom(PlayerCharacter, TargetRoomPos, ExitDirection);
}

void ANextRoomChoiceManager::ClearPendingChoice()
{
	PendingPlayerCharacter = nullptr;
	PendingTargetRoomPos = FIntPoint::ZeroValue;
	PendingExitDirection = EDoorDirection::Up;
	bWaitingForCombatRewardChoice = false;
}
