// Fill out your copyright notice in the Description page of Project Settings.

#include "MapDebugActor.h"
#include "Map/MapLayout.h"
#include "Map/RoomData.h"
#include "Map/RoomActor/DoorDirection.h"
#include "Map/RoomActor/RoomActor.h"
#include "DrawDebugHelpers.h"
#include "Map/MapGenerator.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "GameFramework/Character.h"

AMapDebugActor::AMapDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

ARoomActor* AMapDebugActor::FindSpawnedRoomByGridPos(const FIntPoint& InGridPos) const
{
	for (ARoomActor* Room : SpawnedRooms)
	{
		if (!IsValid(Room))
		{
			continue;
		}

		const FVector RoomLocation = Room->GetActorLocation();
		const FIntPoint RoomGridPos(
			FMath::RoundToInt(RoomLocation.X / RoomSpacing),
			FMath::RoundToInt(RoomLocation.Y / RoomSpacing)
		);

		if (RoomGridPos == InGridPos)
		{
			return Room;
		}
	}

	return nullptr;
}

void AMapDebugActor::RequestMovePlayer(class ACharacter* PlayerCharacter, const FIntPoint& TargetRoomPos, EDoorDirection ExitDirection)
{
	if (!PlayerCharacter)
	{
		return;
	}

	ARoomActor* TargetRoom = FindSpawnedRoomByGridPos(TargetRoomPos);
	if (!TargetRoom)
	{
		return;
	}

	EDoorDirection EntryDirection = EDoorDirection::Up;

	switch (ExitDirection)
	{
	case EDoorDirection::Up:
		EntryDirection = EDoorDirection::Down;
		break;

	case EDoorDirection::Down:
		EntryDirection = EDoorDirection::Up;
		break;

	case EDoorDirection::Left:
		EntryDirection = EDoorDirection::Right;
		break;

	case EDoorDirection::Right:
		EntryDirection = EDoorDirection::Left;
		break;

	default:
		break;
	}

	const FVector TargetLocation = TargetRoom->GetEntrancePoint(EntryDirection);
	PlayerCharacter->SetActorLocation(TargetLocation);
}

void AMapDebugActor::BeginPlay()
{
	Super::BeginPlay();

	MapGenerator = NewObject<UMapGenerator>(this);

	if (bUseRandomSeedOnBeginPlay)
	{
		CurrentSeed = FMath::Rand();
	}

	BindDebugInput();
	GenerateAndDebug();
}

void AMapDebugActor::BindDebugInput()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC) return;

	EnableInput(PC);

	if (InputComponent)
	{
		// R : 새 랜덤 시드로 생성
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AMapDebugActor::RegenerateWithRandomSeed);

		// T : 현재 시드로 다시 생성
		InputComponent->BindKey(EKeys::T, IE_Pressed, this, &AMapDebugActor::RegenerateWithCurrentSeed);
	}
}

void AMapDebugActor::RegenerateWithRandomSeed()
{
	CurrentSeed = FMath::Rand();
	GenerateAndDebug();
}

void AMapDebugActor::RegenerateWithCurrentSeed()
{
	GenerateAndDebug();
}

void AMapDebugActor::SpawnRooms(const FMapLayout& Layout)
{
	if (!RoomActorClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (const TPair<FIntPoint, FRoomData>& Pair : Layout.Rooms)
	{
		const FRoomData& RoomData = Pair.Value;

		const FVector SpawnLocation(
			RoomData.GridPos.X * RoomSpacing,
			RoomData.GridPos.Y * RoomSpacing,
			0.f
		);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		ARoomActor* SpawnedRoom = World->SpawnActor<ARoomActor>(
			RoomActorClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (SpawnedRoom)
		{
			SpawnedRoom->InitializeRoom(RoomData);
			SpawnedRooms.Add(SpawnedRoom);
		}
	}
}

void AMapDebugActor::ClearSpawnedRooms()
{
	for (ARoomActor* Room : SpawnedRooms)
	{
		if (IsValid(Room))
		{
			Room->Destroy();
		}
	}

	SpawnedRooms.Empty();
}

void AMapDebugActor::GenerateAndDebug()
{
	if (!MapGenerator) return;

	FlushPersistentDebugLines(GetWorld());

	FMapGenerationConfig Config;
	Config.RoomCount = CurrentRoomCount;
	Config.Seed = CurrentSeed;

	FMapLayout Layout = MapGenerator->GenerateMap(Config);

	ClearSpawnedRooms();
	SpawnRooms(Layout);

	DebugDrawMap(Layout);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			5.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Current Seed: %d   [R: Random Seed] [T: Rebuild Same Seed]"),
				CurrentSeed
			)
		);
	}
}

void AMapDebugActor::DebugDrawMap(const FMapLayout& Layout)
{
	const float TileSize = 300.f;

	for (const TPair<FIntPoint, FRoomData>& Pair : Layout.Rooms)
	{
		const FRoomData& Room = Pair.Value;

		const FVector WorldPos(
			Room.GridPos.X * TileSize,
			Room.GridPos.Y * TileSize,
			0.f
		);

		FColor Color = FColor::White;

		switch (Room.RoomType)
		{
		case ERoomType::Start:
			Color = FColor::Green;
			break;

		case ERoomType::Boss:
			Color = FColor::Red;
			break;

		case ERoomType::Reward:
			Color = FColor::Yellow;
			break;

		case ERoomType::Shop:
			Color = FColor::Blue;
			break;

		case ERoomType::Event:
			Color = FColor::White;
			break;

		case ERoomType::Normal:
		default:
			Color = FColor::White;
			break;
		}

		DrawDebugBox(GetWorld(), WorldPos, FVector(100.f, 100.f, 50.f), Color, true);
	}
}