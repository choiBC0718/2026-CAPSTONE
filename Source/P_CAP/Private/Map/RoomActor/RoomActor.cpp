// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomActor.h"
#include "Map/RoomActor/DoorActor.h"
#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomActor/Interior/RoomInteriorPropSet.h"
#include "Map/RoomActor/Interior/PCG/RoomPathActor.h"
#include "Map/RoomActor/RoomSizeSettings.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
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
	ClearSpawnedPathActors();
	ClearSpawnedStructureMeshes();
	ClearSpawnedObstacles();
	if (MonsterSpawnerComponent)
	{
		MonsterSpawnerComponent->ClearSpawnedMonsters();
	}

	/* 방 정보 기준으로 문과 경로를 다시 생성 */
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

void ARoomActor::ActivateRoom(AActor* TargetActor)
{
	if (bRoomActivated)
	{
		return;
	}

	bRoomActivated = true;
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
	for (ADoorActor* Door : SpawnedDoors)
	{
		if (IsValid(Door))
		{
			Door->SetPortalEnabled(bEnabled);
		}
	}
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

void ARoomActor::Destroyed()
{
	/* 방이 제거될 때 함께 생성한 객체들도 정리 */
	ClearSpawnedDoors();
	ClearSpawnedPathActors();
	ClearSpawnedStructureMeshes();
	if (MonsterSpawnerComponent)
	{
		MonsterSpawnerComponent->ClearSpawnedMonsters();
	}
	ClearSpawnedObstacles();
	Super::Destroyed();
}

void ARoomActor::ClearSpawnedDoors()
{
	/* 스폰한 문 액터를 모두 제거 */
	for (ADoorActor* Door : SpawnedDoors)
	{
		if (IsValid(Door))
		{
			Door->Destroy();
		}
	}

	SpawnedDoors.Empty();
}

void ARoomActor::ClearSpawnedPathActors()
{
	/* 이 방에서 만든 경로 액터를 모두 제거 */
	for (ARoomPathActor* PathActor : SpawnedPathActors)
	{
		if (IsValid(PathActor))
		{
			PathActor->Destroy();
		}
	}

	SpawnedPathActors.Empty();
}

void ARoomActor::ClearSpawnedStructureMeshes()
{
	for (UStaticMeshComponent* MeshComponent : SpawnedStructureMeshes)
	{
		if (IsValid(MeshComponent))
		{
			MeshComponent->DestroyComponent();
		}
	}

	SpawnedStructureMeshes.Empty();
}

void ARoomActor::SpawnConnectedDoors()
{
	/* 연결 정보가 true인 방향에만 문을 생성 */
	if (CachedRoomData.bConnectedUp)
	{
		SpawnDoor(EDoorDirection::Up);
	}

	if (CachedRoomData.bConnectedDown)
	{
		SpawnDoor(EDoorDirection::Down);
	}

	if (CachedRoomData.bConnectedLeft)
	{
		SpawnDoor(EDoorDirection::Left);
	}

	if (CachedRoomData.bConnectedRight)
	{
		SpawnDoor(EDoorDirection::Right);
	}
}

void ARoomActor::SpawnDoor(EDoorDirection Direction)
{
	if (!DoorActorClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform DoorTransform = GetDoorTransform(Direction);

	/* 항상 스폰해서 방 연결 상태를 눈에 보이게 유지 */
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADoorActor* SpawnedDoor = World->SpawnActor<ADoorActor>(
		DoorActorClass,
		DoorTransform,
		SpawnParams
	);

	if (!SpawnedDoor)
	{
		return;
	}

	const FIntPoint TargetRoomPos = GetNeighborGridPos(Direction);
	/* 현재 방 좌표와 연결 대상 방 좌표를 문 액터에 전달 */
	SpawnedDoor->InitializeDoor(CachedRoomData.GridPos, TargetRoomPos, Direction);

	SpawnedDoors.Add(SpawnedDoor);
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

	/* 생성된 경로 데이터로 실제 path actor를 생성 */
	SpawnGuaranteedPaths(Layout);
	SpawnLargeStructureMeshes(Layout);
	SpawnObstaclesByTendency(CachedTendency);
	DrawInteriorCellDebug(Layout);
}

void ARoomActor::SpawnGuaranteedPaths(const FRoomInteriorLayout& Layout)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector PathOffset(0.f, 0.f, PathZOffset);

	/* 경로 하나마다 path actor 하나를 생성 */
	for (const FRoomInteriorPath& Path : Layout.GuaranteedPaths)
	{
		if (Path.PathPoints.Num() < 2)
		{
			continue;
		}

		TArray<FVector> WorldPathPoints;
		WorldPathPoints.Reserve(Path.PathPoints.Num());

		/* 로컬 경로 점을 월드 좌표로 변환 */
		for (const FVector& PathPoint : Path.PathPoints)
		{
			WorldPathPoints.Add(GetActorTransform().TransformPosition(PathPoint + PathOffset));
		}

		if (WorldPathPoints.Num() < 2)
		{
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		/* 경로를 실제로 들고 있을 전용 액터 생성 */
		ARoomPathActor* SpawnedPathActor = World->SpawnActor<ARoomPathActor>(
			ARoomPathActor::StaticClass(),
			FTransform::Identity,
			SpawnParams
		);

		if (!SpawnedPathActor)
		{
			continue;
		}

		/* 월드 좌표 경로를 spline으로 초기화 */
		SpawnedPathActor->InitializePath(WorldPathPoints, Path.bClosedLoop);
		SpawnedPathActors.Add(SpawnedPathActor);
	}
}

void ARoomActor::SpawnLargeStructureMeshes(const FRoomInteriorLayout& Layout)
{
	if (!LargeStructurePropSet || Layout.CellSize <= 0.f)
	{
		return;
	}

	const float GridWorldSizeX = Layout.GridWidth * Layout.CellSize;
	const float GridWorldSizeY = Layout.GridHeight * Layout.CellSize;
	const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, 0.f);

	int32 Seed = CachedMapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(CachedRoomData.GridPos));
	FRandomStream RandomStream(Seed);

	for (const FRoomInteriorPlacedStructure& Structure : Layout.PlacedStructures)
	{
		const FRoomInteriorPropRule* Rule = LargeStructurePropSet->FindRule(Structure.Category, Structure.Footprint);
		if (!Rule || Rule->Variants.IsEmpty())
		{
			continue;
		}

		int32 TotalWeight = 0;
		for (const FRoomInteriorPropMeshVariant& Variant : Rule->Variants)
		{
			if (Variant.Mesh && Variant.Weight > 0)
			{
				TotalWeight += Variant.Weight;
			}
		}

		if (TotalWeight <= 0)
		{
			continue;
		}

		const int32 RandomPick = RandomStream.RandRange(1, TotalWeight);
		const FRoomInteriorPropMeshVariant* SelectedVariant = nullptr;
		int32 RunningWeight = 0;

		for (const FRoomInteriorPropMeshVariant& Variant : Rule->Variants)
		{
			if (!Variant.Mesh || Variant.Weight <= 0)
			{
				continue;
			}

			RunningWeight += Variant.Weight;
			if (RandomPick <= RunningWeight)
			{
				SelectedVariant = &Variant;
				break;
			}
		}

		if (!SelectedVariant || !SelectedVariant->Mesh)
		{
			continue;
		}

		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this);
		if (!MeshComponent)
		{
			continue;
		}

		const FVector StructureSize(
			Structure.Footprint.X * Layout.CellSize,
			Structure.Footprint.Y * Layout.CellSize,
			0.f
		);
		FVector LocalCenter(
			GridMin.X + (Structure.Origin.X * Layout.CellSize) + (StructureSize.X * 0.5f),
			GridMin.Y + (Structure.Origin.Y * Layout.CellSize) + (StructureSize.Y * 0.5f),
			0.f
		);
		LocalCenter.X += RandomStream.FRandRange(-SelectedVariant->LocationJitter.X, SelectedVariant->LocationJitter.X);
		LocalCenter.Y += RandomStream.FRandRange(-SelectedVariant->LocationJitter.Y, SelectedVariant->LocationJitter.Y);
		LocalCenter.Z += RandomStream.FRandRange(-SelectedVariant->LocationJitter.Z, SelectedVariant->LocationJitter.Z);

		const float UniformScale = RandomStream.FRandRange(
			SelectedVariant->UniformScaleRange.X,
			SelectedVariant->UniformScaleRange.Y);
		const float BaseYaw = Structure.Footprint.X < Structure.Footprint.Y ? 90.f : 0.f;
		const float YawJitter = RandomStream.FRandRange(
			-SelectedVariant->YawJitterDegrees,
			SelectedVariant->YawJitterDegrees);

		MeshComponent->SetupAttachment(Root);
		MeshComponent->SetStaticMesh(SelectedVariant->Mesh);
		MeshComponent->SetRelativeLocation(LocalCenter);
		MeshComponent->SetRelativeRotation(FRotator(0.f, BaseYaw + YawJitter, 0.f));
		MeshComponent->SetRelativeScale3D(FVector(UniformScale));
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->RegisterComponent();
		AddInstanceComponent(MeshComponent);

		SpawnedStructureMeshes.Add(MeshComponent);
	}
}

void ARoomActor::DrawInteriorCellDebug(const FRoomInteriorLayout& Layout) const
{
	if (!bDrawInteriorCellDebug)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || Layout.CellSize <= 0.f)
	{
		return;
	}

	const float CellHalfSize = Layout.CellSize * 0.5f;
	const float GridWorldSizeX = Layout.GridWidth * Layout.CellSize;
	const float GridWorldSizeY = Layout.GridHeight * Layout.CellSize;
	const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, PathZOffset + 60.f);

	for (const FRoomInteriorCell& Cell : Layout.Cells)
	{
		FColor DebugColor;
		switch (Cell.Type)
		{
		case ERoomInteriorCellType::ReservedDoor:
			DebugColor = FColor::Green;
			break;

		case ERoomInteriorCellType::ReservedPath:
			DebugColor = FColor::Blue;
			break;

		case ERoomInteriorCellType::Blocked:
			DebugColor = FColor::Red;
			break;

		default:
			continue;
		}

		const FVector LocalCenter(
			GridMin.X + (Cell.Coord.X * Layout.CellSize) + CellHalfSize,
			GridMin.Y + (Cell.Coord.Y * Layout.CellSize) + CellHalfSize,
			GridMin.Z
		);
		const FVector WorldCenter = GetActorTransform().TransformPosition(LocalCenter);

		DrawDebugBox(
			World,
			WorldCenter,
			FVector(CellHalfSize * 0.45f, CellHalfSize * 0.45f, 35.f),
			GetActorQuat(),
			DebugColor,
			true,
			-1.f,
			1,
			6.f
		);

		DrawDebugSolidBox(
			World,
			WorldCenter,
			FVector(CellHalfSize * 0.42f, CellHalfSize * 0.42f, 25.f),
			GetActorQuat(),
			FColor(DebugColor.R, DebugColor.G, DebugColor.B, 72),
			true,
			-1.f,
			1
		);
	}
}

FTransform ARoomActor::GetDoorTransform(EDoorDirection Direction) const
{
	/* 방향별 로컬 문 위치/회전을 계산 */
	FVector LocalLocation = FVector::ZeroVector;
	FRotator LocalRotation = FRotator::ZeroRotator;
	const float RoomHalfExtentValue = GetEffectiveRoomHalfExtent();
	const float DoorInsetValue = GetEffectiveDoorInset();

	switch (Direction)
	{
	case EDoorDirection::Up:
		LocalLocation = FVector(0.f, RoomHalfExtentValue - DoorInsetValue, DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, 0.f, 0.f);
		break;

	case EDoorDirection::Down:
		LocalLocation = FVector(0.f, -RoomHalfExtentValue + DoorInsetValue, DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, 180.f, 0.f);
		break;

	case EDoorDirection::Left:
		LocalLocation = FVector(-RoomHalfExtentValue + DoorInsetValue, 0.f, DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, -90.f, 0.f);
		break;

	case EDoorDirection::Right:
		LocalLocation = FVector(RoomHalfExtentValue - DoorInsetValue, 0.f, DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, 90.f, 0.f);
		break;

	default:
		break;
	}

	const FVector WorldLocation = GetActorTransform().TransformPosition(LocalLocation);
	const FQuat WorldRotation = GetActorQuat() * LocalRotation.Quaternion();

	/* 방 기준 로컬 값들을 월드 transform으로 변환 */
	return FTransform(WorldRotation, WorldLocation, FVector::OneVector);
}

FIntPoint ARoomActor::GetNeighborGridPos(EDoorDirection Direction) const
{
	/* 문 방향 기준으로 연결 대상 방의 격자 좌표를 계산 */
	switch (Direction)
	{
	case EDoorDirection::Up:
		return CachedRoomData.GridPos + FIntPoint(0, 1);

	case EDoorDirection::Down:
		return CachedRoomData.GridPos + FIntPoint(0, -1);

	case EDoorDirection::Left:
		return CachedRoomData.GridPos + FIntPoint(-1, 0);

	case EDoorDirection::Right:
		return CachedRoomData.GridPos + FIntPoint(1, 0);

	default:
		return CachedRoomData.GridPos;
	}
}

void ARoomActor::ClearSpawnedObstacles()
{
	for (AAnalysisObstacle* Obstacle : SpawnedObstacles)
	{
		if (IsValid(Obstacle))
		{
			Obstacle->Destroy();
		}
	}
	SpawnedObstacles.Empty();
}

void ARoomActor::SpawnObstaclesByTendency(const FPlayerTendencyModifier& Tendency)
{
	// 일반 방에만, ObstacleClass가 지정된 경우에만
	if (!ObstacleClass || MaxObstaclesPerRoom <= 0)
	{
		return;
	}
	if (CachedRoomData.RoomType != ERoomType::Normal)
	{
		return;
	}

	// ObstacleBypass로 최종 장애물 수 결정
	const float BaseCount = FMath::Lerp(1.f, static_cast<float>(MaxObstaclesPerRoom), Tendency.ObstacleBypass);
	const int32 ObstacleCount = FMath::RoundToInt(BaseCount);

	UE_LOG(LogTemp, Log, TEXT("[Obstacle] Room(%d,%d) | Bypass=%.2f → 장애물 %d개"),
		CachedRoomData.GridPos.X, CachedRoomData.GridPos.Y,
		Tendency.ObstacleBypass, ObstacleCount);

	if (ObstacleCount <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 방 시드 기반 랜덤스트림으로 재현 가능한 배치
	int32 Seed = CachedMapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(CachedRoomData.GridPos));
	Seed = HashCombineFast(Seed, 0xF1A20B3C);
	FRandomStream RandomStream(Seed);

	// 문(ReservedDoor) 셀 위치 수집
	TArray<FIntPoint> DoorCoords;
	for (const FRoomInteriorCell& Cell : CachedInteriorLayout.Cells)
	{
		if (Cell.Type == ERoomInteriorCellType::ReservedDoor)
		{
			DoorCoords.Add(Cell.Coord);
		}
	}

	// 스폰 가능 셀 수집 (Empty, 벽 1칸 제외)
	const int32 EdgeMargin = 1;
	TArray<FIntPoint> SpawnableCells;
	for (const FRoomInteriorCell& Cell : CachedInteriorLayout.Cells)
	{
		if (Cell.Type != ERoomInteriorCellType::Empty)
		{
			continue;
		}
		if (Cell.Coord.X < EdgeMargin || Cell.Coord.Y < EdgeMargin ||
			Cell.Coord.X >= CachedInteriorLayout.GridWidth - EdgeMargin ||
			Cell.Coord.Y >= CachedInteriorLayout.GridHeight - EdgeMargin)
		{
			continue;
		}
		SpawnableCells.Add(Cell.Coord);
	}

	// 문까지 최단 거리 계산 후 정규화
	float MaxDoorDist = KINDA_SMALL_NUMBER;
	TMap<FIntPoint, float> DoorDistMap;
	for (const FIntPoint& CellCoord : SpawnableCells)
	{
		float MinDist = TNumericLimits<float>::Max();
		for (const FIntPoint& Door : DoorCoords)
		{
			MinDist = FMath::Min(MinDist, FVector2D::Distance(FVector2D(CellCoord), FVector2D(Door)));
		}
		DoorDistMap.Add(CellCoord, MinDist);
		MaxDoorDist = FMath::Max(MaxDoorDist, MinDist);
	}

	// ObstacleBypass=1 → 문 근처 셀 우선 (점수 낮음)
	// ObstacleBypass=0 → 문에서 먼 셀 우선 (점수 낮음)
	TMap<FIntPoint, float> CellScores;
	for (const FIntPoint& CellCoord : SpawnableCells)
	{
		const float NormalizedDist = DoorDistMap.FindRef(CellCoord) / MaxDoorDist;
		// ExplorationRate 낮음 → 문 근처, 높음 → 구석
		const float DirectionScore = FMath::Lerp(NormalizedDist, 1.f - NormalizedDist, Tendency.ExplorationRate);
		CellScores.Add(CellCoord, DirectionScore + RandomStream.FRandRange(-0.05f, 0.05f));
	}

	SpawnableCells.Sort([&](const FIntPoint& A, const FIntPoint& B)
	{
		return CellScores.FindRef(A) < CellScores.FindRef(B);
	});

	// 셀 로컬 중심 좌표 계산 헬퍼
	const float GridWorldSizeX = CachedInteriorLayout.GridWidth * CachedInteriorLayout.CellSize;
	const float GridWorldSizeY = CachedInteriorLayout.GridHeight * CachedInteriorLayout.CellSize;
	const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, 0.f);
	const float SpawnJitter = CachedInteriorLayout.CellSize * 0.2f;

	int32 CellIndex = 0;
	for (int32 i = 0; i < ObstacleCount; i++)
	{
		// 배치할 셀 탐색 (이미 장애물 있는 셀 건너뜀)
		FVector LocalPos;
		bool bFound = false;

		while (CellIndex < SpawnableCells.Num())
		{
			const FIntPoint& Coord = SpawnableCells[CellIndex++];
			LocalPos = FVector(
				GridMin.X + Coord.X * CachedInteriorLayout.CellSize + CachedInteriorLayout.CellSize * 0.5f + RandomStream.FRandRange(-SpawnJitter, SpawnJitter),
				GridMin.Y + Coord.Y * CachedInteriorLayout.CellSize + CachedInteriorLayout.CellSize * 0.5f + RandomStream.FRandRange(-SpawnJitter, SpawnJitter),
				0.f);

			bool bTooClose = false;
			for (const TObjectPtr<AAnalysisObstacle>& Existing : SpawnedObstacles)
			{
				if (IsValid(Existing) &&
					FVector::Dist(GetActorTransform().TransformPosition(LocalPos), Existing->GetActorLocation()) < CachedInteriorLayout.CellSize)
				{
					bTooClose = true;
					break;
				}
			}

			if (!bTooClose)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			break;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		const FVector WorldPos = GetActorTransform().TransformPosition(LocalPos);
		const FRotator WorldRot = FRotator(0.f, RandomStream.FRandRange(0.f, 360.f), 0.f);

		AAnalysisObstacle* Spawned = World->SpawnActor<AAnalysisObstacle>(ObstacleClass, WorldPos, WorldRot, SpawnParams);
		if (Spawned)
		{
			Spawned->BypassMonsterClass = BypassMonsterClass;
			SpawnedObstacles.Add(Spawned);

			// 문 근처(빨강) vs 구석(파랑) 색상으로 구분
			const float NormalizedScore = CellScores.IsEmpty() ? 0.5f : CellIndex / FMath::Max(1.f, (float)SpawnableCells.Num());
			const FColor DebugColor = NormalizedScore < 0.5f ? FColor::Red : FColor::Blue;
			DrawDebugSphere(World, WorldPos + FVector(0, 0, 150), 80.f, 8, DebugColor, false, 15.f, 0, 3.f);
		}
	}
}


