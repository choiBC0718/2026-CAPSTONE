// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/Debug/MapManager.h"
#include "Map/Debug/MapDebugActor.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Map/RoomTypes.h"
#include "Stage/StageExitActor.h"
#include "Stage/StageDataAsset.h"

AMapManager::AMapManager()
{
	PrimaryActorTick.bCanEverTick = false;
	StageExitActorClass = AStageExitActor::StaticClass();
}

void AMapManager::BeginPlay()
{
	Super::BeginPlay();

	EnsureMapGenerator();

	if (bUseRandomSeedOnBeginPlay)
	{
		CurrentSeed = FMath::Rand();
	}

	BindInput();
	if (bGenerateMapOnBeginPlay)
	{
		GenerateMapAndSpawnRooms();
	}
}

void AMapManager::BindInput()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	EnableInput(PC);

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AMapManager::RegenerateWithRandomSeed);
		InputComponent->BindKey(EKeys::T, IE_Pressed, this, &AMapManager::RegenerateWithCurrentSeed);
	}
}

void AMapManager::RegenerateWithRandomSeed()
{
	CurrentSeed = FMath::Rand();
	GenerateMapAndSpawnRooms();
}

void AMapManager::RegenerateWithCurrentSeed()
{
	GenerateMapAndSpawnRooms();
}

void AMapManager::GenerateStage(const FStageConfig& StageConfig)
{
	CurrentRoomCount = StageConfig.RoomCount;
	CurrentSeed = StageConfig.bUseRandomSeed ? FMath::Rand() : StageConfig.FixedSeed;
	CurrentMonsterSpawnDataAsset = StageConfig.MonsterSpawnDataAsset;

	GenerateMapAndSpawnRooms();
	MovePlayerToStartRoom();
}

void AMapManager::ClearCurrentStage()
{
	ClearSpawnedRooms();
}

void AMapManager::EnsureMapGenerator()
{
	if (!MapGenerator)
	{
		MapGenerator = NewObject<UMapGenerator>(this);
	}
}

void AMapManager::GenerateMapAndSpawnRooms()
{
	EnsureMapGenerator();

	if (!MapGenerator)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		FlushPersistentDebugLines(World);
	}

	FMapGenerationConfig Config;
	Config.RoomCount = CurrentRoomCount;
	Config.Seed = CurrentSeed;

	CurrentLayout = MapGenerator->GenerateMap(Config);

	ClearSpawnedRooms();
	SpawnRooms(CurrentLayout);

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
	
	AMapDebugActor* MapDebugActor = Cast<AMapDebugActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapDebugActor::StaticClass()));

	if (MapDebugActor)
	{
		MapDebugActor->RefreshDebugDraw();
	}
}

void AMapManager::SpawnRooms(const FMapLayout& Layout)
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

		if (!SpawnedRoom)
		{
			continue;
		}

		SpawnedRoom->InitializeRoom(RoomData, Layout.UsedSeed, CurrentMonsterSpawnDataAsset);
		SpawnedRooms.Add(SpawnedRoom);
		SpawnedRoomMap.Add(RoomData.GridPos, SpawnedRoom);

		if (RoomData.RoomType == ERoomType::Boss)
		{
			SpawnStageExitInRoom(SpawnedRoom, RoomData);
		}
	}
}

void AMapManager::SpawnStageExitInRoom(ARoomActor* RoomActor, const FRoomData& RoomData)
{
	if (!RoomActor || !StageExitActorClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation = RoomActor->GetActorTransform().TransformPosition(StageExitLocalOffset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = RoomActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStageExitActor* SpawnedStageExit = World->SpawnActor<AStageExitActor>(
		StageExitActorClass,
		SpawnLocation,
		RoomActor->GetActorRotation(),
		SpawnParams
	);

	if (!SpawnedStageExit)
	{
		return;
	}

	SpawnedStageExit->AttachToActor(RoomActor, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedStageExits.Add(SpawnedStageExit);
}

void AMapManager::MovePlayerToStartRoom()
{
	ARoomActor* StartRoom = FindSpawnedRoomByGridPos(CurrentLayout.StartRoomPos);
	if (!StartRoom)
	{
		return;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter)
	{
		return;
	}

	const FVector StartLocation = StartRoom->GetActorLocation() + FVector(0.f, 0.f, 100.f);
	PlayerCharacter->SetActorLocation(StartLocation);
}

void AMapManager::ClearSpawnedStageExits()
{
	for (AStageExitActor* StageExit : SpawnedStageExits)
	{
		if (IsValid(StageExit))
		{
			StageExit->Destroy();
		}
	}

	SpawnedStageExits.Empty();
}

void AMapManager::ClearSpawnedRooms()
{
	ClearSpawnedStageExits();

	for (ARoomActor* Room : SpawnedRooms)
	{
		if (IsValid(Room))
		{
			Room->Destroy();
		}
	}

	SpawnedRooms.Empty();
	SpawnedRoomMap.Empty();
}

ARoomActor* AMapManager::FindSpawnedRoomByGridPos(const FIntPoint& InGridPos) const
{
	if (const TObjectPtr<ARoomActor>* FoundRoom = SpawnedRoomMap.Find(InGridPos))
	{
		return FoundRoom->Get();
	}

	return nullptr;
}

void AMapManager::RequestMovePlayer(ACharacter* PlayerCharacter, const FIntPoint& TargetRoomPos, EDoorDirection ExitDirection)
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

