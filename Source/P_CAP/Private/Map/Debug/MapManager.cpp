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
#include "Map/NextRoomChoiceManager.h"
#include "Map/RoomActor/RoomSizeSettings.h"
#include "Stage/StageExitActor.h"
#include "Stage/StageDataAsset.h"
#include "UObject/ConstructorHelpers.h"
#include "Framework/CAP_GameInstance.h"

AMapManager::AMapManager()
{
	PrimaryActorTick.bCanEverTick = false;
	StageExitActorClass = AStageExitActor::StaticClass();

	static ConstructorHelpers::FClassFinder<ANextRoomChoiceManager> ChoiceManagerClassFinder(
		TEXT("/Game/_Workspace/6_Room/ChoiceWidget/BP_NextRoomChoiceManager"));
	if (ChoiceManagerClassFinder.Succeeded())
	{
		NextRoomChoiceManagerClass = ChoiceManagerClassFinder.Class;
	}
	else
	{
		NextRoomChoiceManagerClass = ANextRoomChoiceManager::StaticClass();
	}
}

void AMapManager::BeginPlay()
{
	Super::BeginPlay();

	EnsureMapGenerator();
	EnsureNextRoomChoiceManager();

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

float AMapManager::GetRoomSpacing() const
{
	return GetEffectiveRoomSpacing();
}

void AMapManager::EnsureNextRoomChoiceManager()
{
	if (NextRoomChoiceManager)
	{
		return;
	}

	NextRoomChoiceManager = Cast<ANextRoomChoiceManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ANextRoomChoiceManager::StaticClass()));

	if (NextRoomChoiceManager || !NextRoomChoiceManagerClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	NextRoomChoiceManager = World->SpawnActor<ANextRoomChoiceManager>(
		NextRoomChoiceManagerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams);
}

float AMapManager::GetEffectiveRoomSpacing() const
{
	return RoomSizeSettings ? RoomSizeSettings->GetRoomSpacing() : RoomSpacing;
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

	// 성향 결정: 수동 오버라이드 → Assessment → K-Means 순서로 우선순위 적용
	FPlayerTendencyModifier Tendency;
	if (bUseManualTendency)
	{
		Tendency = ManualTendency;
		UE_LOG(LogTemp, Warning, TEXT("MapManager: 수동 성향 적용 — Combat=%.2f, Explore=%.2f, Obstacle=%.2f"),
			Tendency.CombatAggression, Tendency.ExplorationRate, Tendency.ObstacleBypass);
	}
	else
	{
		bool bApplied = false;
		if (UCAP_GameInstance* GI = Cast<UCAP_GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (GI->HasAssessmentTendency())
			{
				Tendency = GI->GetAssessmentTendency();
				UE_LOG(LogTemp, Warning, TEXT("MapManager: Assessment 성향 적용 — Combat=%.2f, Explore=%.2f, Obstacle=%.2f"),
					Tendency.CombatAggression, Tendency.ExplorationRate, Tendency.ObstacleBypass);
				bApplied = true;
			}
		}
		if (!bApplied)
		{
			APlayerBehaviorLearner* Learner = Cast<APlayerBehaviorLearner>(
				UGameplayStatics::GetActorOfClass(World, APlayerBehaviorLearner::StaticClass()));
			if (Learner)
			{
				Tendency = Learner->GetCurrentPlayerTendency();
				UE_LOG(LogTemp, Log, TEXT("MapManager: K-Means 성향 적용 — Combat=%.2f, Explore=%.2f, Obstacle=%.2f"),
					Tendency.CombatAggression, Tendency.ExplorationRate, Tendency.ObstacleBypass);
			}
		}
	}
	LastAppliedTendency = Tendency;
	if (!bUseManualTendency)
	{
		ManualTendency = Tendency;
	}

	// 맵 전체 중심 계산
	FVector2D Centroid(0.f, 0.f);
	for (const TPair<FIntPoint, FRoomData>& Pair : Layout.Rooms)
	{
		Centroid += FVector2D(Pair.Key.X, Pair.Key.Y);
	}
	Centroid /= FMath::Max(1, Layout.Rooms.Num());

	// 중심에서 가장 먼 방까지의 거리 (구역 경계 정규화용)
	float MaxDist = KINDA_SMALL_NUMBER;
	for (const TPair<FIntPoint, FRoomData>& Pair : Layout.Rooms)
	{
		const float D = FVector2D::Distance(FVector2D(Pair.Key.X, Pair.Key.Y), Centroid);
		if (D > MaxDist) MaxDist = D;
	}

	for (const TPair<FIntPoint, FRoomData>& Pair : Layout.Rooms)
	{
		const FRoomData& RoomData = Pair.Value;
		const float EffectiveRoomSpacing = GetEffectiveRoomSpacing();

		const FVector SpawnLocation(
			RoomData.GridPos.X * EffectiveRoomSpacing,
			RoomData.GridPos.Y * EffectiveRoomSpacing,
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

		// 맵 중심 기준 공간 거리 비율로 구역 분류
		// Core(0~33%) = 중앙, Mid(33~66%) = 중간, Outer(66~100%) = 외곽
		const float DistRatio = FVector2D::Distance(FVector2D(RoomData.GridPos.X, RoomData.GridPos.Y), Centroid) / MaxDist;
		ERoomZone Zone = ERoomZone::Outer;
		if (DistRatio < 0.33f) Zone = ERoomZone::Core;
		else if (DistRatio < 0.66f) Zone = ERoomZone::Mid;

		SpawnedRoom->InitializeRoom(RoomData, Layout.UsedSeed, CurrentMonsterSpawnDataAsset, Tendency, Zone, RoomSizeSettings);
		SpawnedRooms.Add(SpawnedRoom);
		SpawnedRoomMap.Add(RoomData.GridPos, SpawnedRoom);

		if (RoomData.RoomType == ERoomType::Boss)
		{
			if (bSpawnStageExitActor)
			{
				SpawnStageExitInRoom(SpawnedRoom, RoomData);
			}
			else
			{
				SpawnBossRoomTemporaryActor(SpawnedRoom);
			}
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

	const FTransform SpawnTransform = RoomActor->GetStageExitSpawnTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = RoomActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStageExitActor* SpawnedStageExit = World->SpawnActor<AStageExitActor>(
		StageExitActorClass,
		SpawnTransform.GetLocation(),
		SpawnTransform.GetRotation().Rotator(),
		SpawnParams
	);

	if (!SpawnedStageExit)
	{
		return;
	}

	SpawnedStageExit->AttachToActor(RoomActor, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedStageExits.Add(SpawnedStageExit);
}

void AMapManager::SpawnBossRoomTemporaryActor(ARoomActor* RoomActor)
{
	if (!RoomActor || !BossRoomTemporaryActorClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation = RoomActor->GetActorTransform().TransformPosition(BossRoomTemporaryActorLocalOffset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = RoomActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(
		BossRoomTemporaryActorClass,
		SpawnLocation,
		RoomActor->GetActorRotation(),
		SpawnParams);

	if (!SpawnedActor)
	{
		return;
	}

	SpawnedActor->AttachToActor(RoomActor, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedBossRoomTemporaryActors.Add(SpawnedActor);
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

	for (AActor* TemporaryActor : SpawnedBossRoomTemporaryActors)
	{
		if (IsValid(TemporaryActor))
		{
			TemporaryActor->Destroy();
		}
	}

	SpawnedBossRoomTemporaryActors.Empty();
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

FRoomData* AMapManager::FindRoomData(const FIntPoint& InGridPos)
{
	return CurrentLayout.FindRoom(InGridPos);
}

const FRoomData* AMapManager::FindRoomData(const FIntPoint& InGridPos) const
{
	return CurrentLayout.FindRoom(InGridPos);
}

void AMapManager::MovePlayerToRoom(ACharacter* PlayerCharacter, const FIntPoint& TargetRoomPos, EDoorDirection ExitDirection)
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

void AMapManager::RequestMovePlayer(ACharacter* PlayerCharacter, const FIntPoint& TargetRoomPos, EDoorDirection ExitDirection)
{
	MovePlayerToRoom(PlayerCharacter, TargetRoomPos, ExitDirection);
}

