// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomActor.h"
#include "Map/RoomActor/DoorActor.h"
#include "Map/RoomActor/RoomDoors.h"
#include "Map/RoomActor/RoomFence.h"
#include "Map/RoomActor/RoomFloor.h"
#include "Map/RoomActor/RoomObstacle.h"
#include "Map/RoomActor/RoomStructure.h"
#include "Map/RoomActor/RoomTemplate.h"
#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomActor/Interior/RoomInteriorPropSet.h"
#include "Map/RoomActor/Interior/RoomInteriorTemplateActor.h"
#include "Map/RoomActor/RoomSizeSettings.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Framework/CAP_RewardSettings.h"
#include "Framework/Subsystem/CAP_RewardSubsystem.h"
#include "Interactables/Reward/CAP_RewardChest.h"

ARoomActor::ARoomActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	/* 방의 기준이 되는 루트 */
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	/* 바닥 메시 */
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(Root);

	StageExitSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("StageExitSpawnPoint"));
	StageExitSpawnPoint->SetupAttachment(Root);
	StageExitSpawnPoint->SetRelativeLocation(FVector(0.f, 0.f, 120.f));

	RoomEnterTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("RoomEnterTrigger"));
	RoomEnterTrigger->SetupAttachment(Root);
	RoomEnterTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RoomEnterTrigger->SetCollisionObjectType(ECC_WorldDynamic);
	RoomEnterTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	RoomEnterTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RoomEnterTrigger->SetGenerateOverlapEvents(true);

	/* 몬스터 스폰 전용 컴포넌트 */
	MonsterSpawnerComponent = CreateDefaultSubobject<URoomMonsterSpawnerComponent>(TEXT("MonsterSpawnerComponent"));
}

void ARoomActor::BeginPlay()
{
	Super::BeginPlay();

	UpdateRoomEnterTriggerExtent();
	if (RoomEnterTrigger)
	{
		RoomEnterTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARoomActor::OnRoomEnterTriggerBeginOverlap);
	}

	CheckPlayerInsideRoom();
	SetActorTickEnabled(true);
}

void ARoomActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckPlayerInsideRoom();
	if (bRoomActivated)
	{
		CheckRoomClear();
	}
}

void ARoomActor::SpawnRewardChest()
{
	if (!RewardChestClass)
	{
		UE_LOG(LogTemp,Warning,TEXT("RoomActor에 RewardChestClass 설정 필요"));
		return;
	}
	
	FVector SpawnLoc = GetActorTransform().GetLocation() + FVector(0.f, 0.f, 150.f);
	FRotator SpawnRot = FRotator::ZeroRotator;
	FTransform SpawnTransform(SpawnRot, SpawnLoc);
	
	EChestGrade SelectedGrade = EChestGrade::Normal;
	if (UGameInstance* GI = GetGameInstance())
		if (UCAP_RewardSubsystem* RewardSubsys = GI->GetSubsystem<UCAP_RewardSubsystem>())
			SelectedGrade = RewardSubsys->GetNextChestGrade(CachedRoomData.CombatRewardType);

	ERewardChestType SelectedType = ERewardChestType::Item;
	switch (CachedRoomData.CombatRewardType)
	{
	case ECombatRoomRewardType::Gold:
		SelectedType = ERewardChestType::Gold;		break;
	case ECombatRoomRewardType::Item:
		SelectedType = ERewardChestType::Item;		break;
	case ECombatRoomRewardType::Weapon:
		SelectedType = ERewardChestType::Weapon;	break;
	default:
		SelectedType = ERewardChestType::Item;		break;
	}
	
	if (ACAP_RewardChest* RewardChest = GetWorld()->SpawnActorDeferred<ACAP_RewardChest>(RewardChestClass, SpawnTransform))
	{
		RewardChest->ChestType = SelectedType;
		RewardChest->ChestGrade = SelectedGrade;
		RewardChest->FinishSpawning(SpawnTransform);
	}
}

void ARoomActor::InitializeRoom(
	const FRoomData& InRoomData,
	int32 InMapSeed,
	URoomMonsterSpawnDataAsset* InMonsterSpawnDataAsset,
	const FPlayerTendencyModifier& InTendency,
	ERoomZone InZone,
	URoomSizeSettings* InRoomSizeSettings)
{
	/* 현재 방 정보와 맵 시드, 성향 데이터를 캐싱 */
	CachedRoomData = InRoomData;
	CachedMapSeed = InMapSeed;
	CachedTendency = InTendency;
	CachedZone = InZone;
	CachedRoomSizeSettings = InRoomSizeSettings;
	bRoomActivated = false;
	bRoomCleared = false;
	bMonstersSpawned = false;
	ApplyFloorMeshScale();
	UpdateRoomEnterTriggerExtent();

	if (MonsterSpawnerComponent)
	{
		MonsterSpawnerComponent->SetSpawnDataAsset(InMonsterSpawnDataAsset);
	}

	/* 재초기화 상황을 대비해 기존 문/경로/장애물 액터를 정리 */
	ClearSpawnedDoors();
	FRoomStructure::Clear(*this);
	ClearSpawnedInteriorTemplate();
	ClearSpawnedVisualFloorTiles();
	ClearSpawnedEdgeFences();
	ClearSpawnedObstacles();
	SelectedInteriorTemplateClass = nullptr;
	if (MonsterSpawnerComponent)
	{
		MonsterSpawnerComponent->ClearSpawnedMonsters();
	}

	/* 방 정보 기준으로 문과 경로를 다시 생성 */
	SelectInteriorTemplateClass();
	SpawnInteriorTemplate();
	SpawnVisualFloorTiles();
	SpawnEdgeFences();
	ApplyBaseFloorVisibility();
	SpawnConnectedDoors();
	GenerateAndSpawnInterior();
	SetSpawnedDoorsPortalEnabled(true);

	if (UWorld* World = GetWorld())
	{
		FTimerHandle CheckPlayerInsideTimerHandle;
		World->GetTimerManager().SetTimer(
			CheckPlayerInsideTimerHandle,
			this,
			&ARoomActor::CheckPlayerInsideRoom,
			0.1f,
			false);
	}
}

void ARoomActor::SetCombatRewardType(ECombatRoomRewardType NewRewardType)
{
	CachedRoomData.CombatRewardType = NewRewardType;
}

void ARoomActor::ApplyPersistentClearedState()
{
	bRoomCleared = true;
	bMonstersSpawned = true;

	if (MonsterSpawnerComponent)
	{
		MonsterSpawnerComponent->ClearSpawnedMonsters();
	}

	SetSpawnedDoorsPortalEnabled(true);
	SetActorTickEnabled(false);
}

void ARoomActor::ActivateRoom(AActor* TargetActor)
{
	if (bRoomActivated)
	{
		return;
	}

	bRoomActivated = true;

	if (bRoomCleared)
	{
		SetSpawnedDoorsPortalEnabled(true);
		SetActorTickEnabled(false);
		return;
	}

	SpawnRoomMonsters();
	if (MonsterSpawnerComponent)
	{
		MonsterSpawnerComponent->ActivateSpawnedMonsters(TargetActor);
	}

	if (ShouldLockPortalsForCombat())
	{
		SetSpawnedDoorsPortalEnabled(false);
		SetActorTickEnabled(true);
	}
	else
	{
		bRoomCleared = true;
		SetSpawnedDoorsPortalEnabled(true);
	}
}

void ARoomActor::DeactivateRoom()
{
	if (!bRoomActivated)
	{
		return;
	}

	bRoomActivated = false;
	if (MonsterSpawnerComponent)
	{
		MonsterSpawnerComponent->DeactivateSpawnedMonsters();
	}

	SetSpawnedDoorsPortalEnabled(true);
}

void ARoomActor::SpawnRoomMonsters()
{
	if (bMonstersSpawned || !MonsterSpawnerComponent)
	{
		return;
	}

	bMonstersSpawned = true;
	MonsterSpawnerComponent->SpawnMonsters(
		CachedRoomData,
		CachedInteriorLayout,
		CachedMapSeed,
		GetActorTransform(),
		CachedTendency);
}

void ARoomActor::OnRoomEnterTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || bRoomActivated)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (OtherActor == PlayerPawn)
	{
		ActivateRoom(OtherActor);
	}
}

void ARoomActor::UpdateRoomEnterTriggerExtent()
{
	if (!RoomEnterTrigger)
	{
		return;
	}

	const float TriggerHalfExtent = GetEffectiveTriggerHalfExtent();
	RoomEnterTrigger->SetBoxExtent(FVector(TriggerHalfExtent, TriggerHalfExtent, RoomEnterTriggerHeight));
	RoomEnterTrigger->SetRelativeLocation(FVector(0.f, 0.f, RoomEnterTriggerHeight));
}

float ARoomActor::GetEffectiveRoomHalfExtent() const
{
	return CachedRoomSizeSettings ? CachedRoomSizeSettings->RoomHalfExtent : RoomHalfExtent;
}

float ARoomActor::GetEffectiveDoorInset() const
{
	return CachedRoomSizeSettings ? CachedRoomSizeSettings->DoorInset : DoorInset;
}

float ARoomActor::GetEffectiveEntranceInset() const
{
	return CachedRoomSizeSettings ? CachedRoomSizeSettings->EntranceInset : 200.f;
}

float ARoomActor::GetEffectiveTriggerHalfExtent() const
{
	return CachedRoomSizeSettings
		? CachedRoomSizeSettings->GetTriggerHalfExtent()
		: FMath::Max(100.f, RoomHalfExtent - 50.f);
}

float ARoomActor::GetEffectiveInteriorCellSize() const
{
	return CachedRoomSizeSettings ? CachedRoomSizeSettings->InteriorCellSize : InteriorCellSize;
}

float ARoomActor::GetEffectiveInteriorMargin() const
{
	return CachedRoomSizeSettings ? CachedRoomSizeSettings->InteriorMargin : InteriorMargin;
}

void ARoomActor::ApplyFloorMeshScale()
{
	if (!FloorMesh || !CachedRoomSizeSettings || !CachedRoomSizeSettings->bAutoScaleFloorMesh)
	{
		return;
	}

	if (!bHasInitialFloorMeshScale)
	{
		InitialFloorMeshScale = FloorMesh->GetRelativeScale3D();
		bHasInitialFloorMeshScale = true;
	}

	const float FloorScale = CachedRoomSizeSettings->GetFloorScale();
	FloorMesh->SetRelativeScale3D(FVector(
		InitialFloorMeshScale.X * FloorScale,
		InitialFloorMeshScale.Y * FloorScale,
		InitialFloorMeshScale.Z));
}

void ARoomActor::CheckPlayerInsideRoom()
{
	if (bRoomActivated)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	if (RoomEnterTrigger)
	{
		RoomEnterTrigger->UpdateOverlaps();
		if (RoomEnterTrigger->IsOverlappingActor(PlayerPawn))
		{
			ActivateRoom(PlayerPawn);
			return;
		}
	}

	const FVector LocalPlayerLocation = GetActorTransform().InverseTransformPosition(PlayerPawn->GetActorLocation());
	const float TriggerHalfExtent = GetEffectiveTriggerHalfExtent();
	if (FMath::Abs(LocalPlayerLocation.X) <= TriggerHalfExtent &&
		FMath::Abs(LocalPlayerLocation.Y) <= TriggerHalfExtent)
	{
		ActivateRoom(PlayerPawn);
	}
}

void ARoomActor::CheckRoomClear()
{
	if (bRoomCleared || !ShouldLockPortalsForCombat())
	{
		SetActorTickEnabled(false);
		return;
	}

	if (MonsterSpawnerComponent && MonsterSpawnerComponent->AreAllSpawnedMonstersDefeated())
	{
		bRoomCleared = true;
		HandleCombatRoomCleared();
		SetSpawnedDoorsPortalEnabled(true);
		SetActorTickEnabled(false);
		SpawnRewardChest();
	}
}

bool ARoomActor::ShouldLockPortalsForCombat() const
{
	return CachedRoomData.RoomType == ERoomType::Normal &&
		MonsterSpawnerComponent &&
		MonsterSpawnerComponent->HasSpawnedMonsters();
}

void ARoomActor::HandleCombatRoomCleared()
{
	if (CachedRoomData.RoomType != ERoomType::Normal)
	{
		return;
	}

	const TCHAR* RewardTypeText = TEXT("None");
	switch (CachedRoomData.CombatRewardType)
	{
	case ECombatRoomRewardType::Gold:
		RewardTypeText = TEXT("Gold");
		break;

	case ECombatRoomRewardType::Item:
		RewardTypeText = TEXT("Item");
		break;

	case ECombatRoomRewardType::Weapon:
		RewardTypeText = TEXT("Weapon");
		break;

	case ECombatRoomRewardType::None:
	default:
		break;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("RoomActor: combat room cleared at (%d, %d), reward type = %s"),
		CachedRoomData.GridPos.X,
		CachedRoomData.GridPos.Y,
		RewardTypeText);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Green,
			FString::Printf(TEXT("Combat reward ready: %s"), RewardTypeText));
	}
}

void ARoomActor::SetSpawnedDoorsPortalEnabled(bool bEnabled)
{
	FRoomDoors::SetPortalEnabled(*this, bEnabled);
}

FVector ARoomActor::GetEntrancePoint(EDoorDirection Direction) const
{
	const float RoomHalfExtentValue = GetEffectiveRoomHalfExtent();
	const float EntranceInsetValue = GetEffectiveEntranceInset();

	FVector LocalLocation = FVector::ZeroVector;

	switch (Direction)
	{
	case EDoorDirection::Up:
		LocalLocation = FVector(0.f, RoomHalfExtentValue - EntranceInsetValue, 0.f);
		break;

	case EDoorDirection::Down:
		LocalLocation = FVector(0.f, -RoomHalfExtentValue + EntranceInsetValue, 0.f);
		break;

	case EDoorDirection::Left:
		LocalLocation = FVector(-RoomHalfExtentValue + EntranceInsetValue, 0.f, 0.f);
		break;

	case EDoorDirection::Right:
		LocalLocation = FVector(RoomHalfExtentValue - EntranceInsetValue, 0.f, 0.f);
		break;

	default:
		break;
	}

	return GetActorTransform().TransformPosition(LocalLocation);
}

FTransform ARoomActor::GetStageExitSpawnTransform() const
{
	return StageExitSpawnPoint ? StageExitSpawnPoint->GetComponentTransform() : GetActorTransform();
}

void ARoomActor::Destroyed()
{
	/* 방이 제거될 때 함께 생성한 객체들도 정리 */
	ClearSpawnedDoors();
	FRoomStructure::Clear(*this);
	ClearSpawnedInteriorTemplate();
	ClearSpawnedVisualFloorTiles();
	ClearSpawnedEdgeFences();
	if (MonsterSpawnerComponent)
	{
		MonsterSpawnerComponent->ClearSpawnedMonsters();
	}
	ClearSpawnedObstacles();
	Super::Destroyed();
}

void ARoomActor::ClearSpawnedDoors()
{
	FRoomDoors::Clear(*this);
}

void ARoomActor::ClearSpawnedInteriorTemplate()
{
	FRoomTemplate::Clear(*this);
}

void ARoomActor::ClearSpawnedVisualFloorTiles()
{
	FRoomFloor::ClearVisualTiles(*this);
}

void ARoomActor::ClearSpawnedEdgeFences()
{
	FRoomFence::Clear(*this);
}

void ARoomActor::SpawnConnectedDoors()
{
	FRoomDoors::SpawnConnected(*this);
}

void ARoomActor::SpawnDoor(EDoorDirection Direction)
{
	FRoomDoors::Spawn(*this, Direction);
}

void ARoomActor::GenerateAndSpawnInterior()
{
	/* 내부 경로 생성기 준비 */
	if (!InteriorGenerator)
	{
		InteriorGenerator = NewObject<URoomInteriorGenerator>(this);
	}

	if (!InteriorGenerator)
	{
		return;
	}

	const FRoomInteriorLayout Layout = InteriorGenerator->GenerateInteriorLayout(
		CachedRoomData,
		GetEffectiveRoomHalfExtent(),
		GetEffectiveInteriorCellSize(),
		GetEffectiveInteriorMargin(),
		CachedMapSeed);
	CachedInteriorLayout = Layout;

	if (!bUseInteriorTemplates || !bDisableGeneratedLargeStructuresWhenUsingTemplate)
	{
		FRoomStructure::SpawnLargeMeshes(*this, Layout);
	}
	SpawnObstaclesByTendency(CachedTendency);
	FRoomStructure::DrawDebugCells(*this, Layout);
}

void ARoomActor::SelectInteriorTemplateClass()
{
	FRoomTemplate::Select(*this);
}

void ARoomActor::SpawnInteriorTemplate()
{
	FRoomTemplate::Spawn(*this);
}

void ARoomActor::SpawnVisualFloorTiles()
{
	FRoomFloor::SpawnVisualTiles(*this);
}

bool ARoomActor::ShouldSkipVisualFloorTileAtLocalBounds(const FVector& LocalCenter, const FVector& LocalExtent) const
{
	return FRoomFloor::ShouldSkipVisualTileAtLocalBounds(*this, LocalCenter, LocalExtent);
}

void ARoomActor::ApplyBaseFloorVisibility()
{
	FRoomFloor::ApplyBaseVisibility(*this);
}

void ARoomActor::SpawnEdgeFences()
{
	FRoomFence::Spawn(*this);
}

FTransform ARoomActor::GetDoorTransform(EDoorDirection Direction) const
{
	return FRoomDoors::GetTransform(*this, Direction);
}

FIntPoint ARoomActor::GetNeighborGridPos(EDoorDirection Direction) const
{
	return FRoomDoors::GetNeighborGridPos(*this, Direction);
}

void ARoomActor::ClearSpawnedObstacles()
{
	FRoomObstacle::Clear(*this);
}

void ARoomActor::SpawnObstaclesByTendency(const FPlayerTendencyModifier& Tendency)
{
	FRoomObstacle::SpawnByTendency(*this, Tendency);
}


