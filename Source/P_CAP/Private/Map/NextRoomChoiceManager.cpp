// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/NextRoomChoiceManager.h"

#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Map/RoomActor/RoomActor.h"
#include "Kismet/GameplayStatics.h"
#include "Map/Debug/MapManager.h"
#include "Map/RoomData.h"
#include "Map/Widget/CombatRewardChoiceWidget.h"

ANextRoomChoiceManager::ANextRoomChoiceManager()
{
	PrimaryActorTick.bCanEverTick = false;
	DefaultChoiceRowNames = { TEXT("Gold"), TEXT("Item"), TEXT("Weapon") };
}

void ANextRoomChoiceManager::BeginPlay()
{
	Super::BeginPlay();

	ResolveMapManager();
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

void ANextRoomChoiceManager::CancelCombatRewardChoiceForRoom(const FIntPoint& TargetRoomPos)
{
	if (!bWaitingForCombatRewardChoice || PendingTargetRoomPos != TargetRoomPos)
	{
		return;
	}

	ClearPendingChoice();
}

void ANextRoomChoiceManager::SelectGoldReward()
{
	SelectReward(ECombatRoomRewardType::Gold);
}

void ANextRoomChoiceManager::SelectItemReward()
{
	SelectReward(ECombatRoomRewardType::Item);
}

void ANextRoomChoiceManager::SelectReward(ECombatRoomRewardType RewardType)
{
	ApplyCombatRewardChoice(RewardType);
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

bool ANextRoomChoiceManager::DoesRoomNeedCombatRewardChoice(const FRoomData& RoomData) const
{
	return RoomData.RoomType == ERoomType::Normal &&
		RoomData.CombatRewardType == ECombatRoomRewardType::None;
}

TArray<FCombatRewardChoiceOption> ANextRoomChoiceManager::BuildCombatRewardChoiceOptions() const
{
	TArray<FCombatRewardChoiceOption> Options;
	TArray<FName> RowNames = DefaultChoiceRowNames;

	if (RowNames.IsEmpty())
	{
		RowNames = { TEXT("Gold"), TEXT("Item"), TEXT("Weapon") };
	}

	auto MakeFallbackOption = [](ECombatRoomRewardType RewardType) -> FCombatRewardChoiceOption
	{
		FCombatRewardChoiceOption Option;
		Option.RewardType = RewardType;
		Option.BadgeText = FText::FromString(TEXT("Reward"));

		switch (RewardType)
		{
		case ECombatRoomRewardType::Gold:
			Option.Title = FText::FromString(TEXT("Gold"));
			Option.Description = FText::FromString(TEXT("전투 클리어 시 골드를 획득합니다."));
			Option.SortOrder = 0;
			break;

		case ECombatRoomRewardType::Item:
			Option.Title = FText::FromString(TEXT("Item"));
			Option.Description = FText::FromString(TEXT("전투 클리어 시 아이템을 획득합니다."));
			Option.SortOrder = 1;
			break;

		case ECombatRoomRewardType::Weapon:
			Option.Title = FText::FromString(TEXT("Weapon"));
			Option.Description = FText::FromString(TEXT("전투 클리어 시 무기를 획득합니다."));
			Option.SortOrder = 2;
			break;

		default:
			Option.Title = FText::FromString(TEXT("Unknown"));
			Option.Description = FText::FromString(TEXT("Unknown reward."));
			Option.SortOrder = 999;
			break;
		}

		return Option;
	};

	for (const FName& RowName : RowNames)
	{
		if (RowName.IsNone())
		{
			continue;
		}

		if (CombatRewardChoiceTable)
		{
			if (const FCombatRewardChoiceOption* Row = CombatRewardChoiceTable->FindRow<FCombatRewardChoiceOption>(
				RowName,
				TEXT("BuildCombatRewardChoiceOptions")))
			{
				Options.Add(*Row);
				continue;
			}
		}

		if (RowName == TEXT("Gold"))
		{
			Options.Add(MakeFallbackOption(ECombatRoomRewardType::Gold));
		}
		else if (RowName == TEXT("Item"))
		{
			Options.Add(MakeFallbackOption(ECombatRoomRewardType::Item));
		}
		else if (RowName == TEXT("Weapon"))
		{
			Options.Add(MakeFallbackOption(ECombatRoomRewardType::Weapon));
		}
	}

	Options.Sort([](const FCombatRewardChoiceOption& Left, const FCombatRewardChoiceOption& Right)
	{
		return Left.SortOrder < Right.SortOrder;
	});

	return Options;
}

bool ANextRoomChoiceManager::ShowChoiceWidget()
{
	if (ActiveChoiceWidget || !ChoiceWidgetClass)
	{
		return false;
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return false;
	}

	ActiveChoiceWidget = CreateWidget<UCombatRewardChoiceWidget>(PC, ChoiceWidgetClass);
	if (!ActiveChoiceWidget)
	{
		return false;
	}

	ActiveChoiceWidget->InitializeChoiceWidget(this, CurrentChoiceOptions);
	ActiveChoiceWidget->AddToViewport();

	/*
	PC->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(ActiveChoiceWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	*/
	return true;
}

void ANextRoomChoiceManager::HideChoiceWidget()
{
	if (!ActiveChoiceWidget)
	{
		return;
	}

	ActiveChoiceWidget->CloseChoiceWidget();
	ActiveChoiceWidget = nullptr;

	/*
	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
	*/
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
	CurrentChoiceOptions = BuildCombatRewardChoiceOptions();
	bWaitingForCombatRewardChoice = true;
	if (!ShowChoiceWidget())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("NextRoomChoiceManager: ChoiceWidgetClass is missing. Moving to room without reward choice."));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				4.0f,
				FColor::Red,
				TEXT("Choice widget missing: moved without reward choice"));
		}

		ClearPendingChoice();
		if (AMapManager* ResolvedMapManager = ResolveMapManager())
		{
			ResolvedMapManager->MovePlayerToRoom(PlayerCharacter, TargetRoomPos, ExitDirection);
		}
		return;
	}
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
			TEXT("Choose combat reward"));
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
	HideChoiceWidget();
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
	HideChoiceWidget();
	PendingPlayerCharacter = nullptr;
	PendingTargetRoomPos = FIntPoint::ZeroValue;
	PendingExitDirection = EDoorDirection::Up;
	CurrentChoiceOptions.Empty();
	bWaitingForCombatRewardChoice = false;
}
