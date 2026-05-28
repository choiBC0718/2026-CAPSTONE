// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomActor.h"
#include "Map/RoomActor/DoorActor.h"
#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomActor/Interior/RoomInteriorPropSet.h"
#include "Map/RoomActor/Interior/PCG/RoomPathActor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

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

void ARoomActor::InitializeRoom(
	const FRoomData& InRoomData,
	int32 InMapSeed,
	URoomMonsterSpawnDataAsset* InMonsterSpawnDataAsset,
	const FPlayerTendencyModifier& InTendency)
{
	/* 현재 방 정보와 맵 시드, 성향 데이터를 캐싱 */
	CachedRoomData = InRoomData;
	CachedMapSeed = InMapSeed;
	CachedTendency = InTendency;
	bRoomActivated = false;
	bRoomCleared = false;
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

void ARoomActor::ActivateRoom(AActor* TargetActor)
{
	if (bRoomActivated)
	{
		return;
	}

	bRoomActivated = true;
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

	const float TriggerHalfExtent = FMath::Max(100.f, RoomHalfExtent - 50.f);
	RoomEnterTrigger->SetBoxExtent(FVector(TriggerHalfExtent, TriggerHalfExtent, RoomEnterTriggerHeight));
	RoomEnterTrigger->SetRelativeLocation(FVector(0.f, 0.f, RoomEnterTriggerHeight));
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
	const float TriggerHalfExtent = FMath::Max(100.f, RoomHalfExtent - 50.f);
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
		SetSpawnedDoorsPortalEnabled(true);
		SetActorTickEnabled(false);
	}
}

bool ARoomActor::ShouldLockPortalsForCombat() const
{
	return CachedRoomData.RoomType == ERoomType::Normal &&
		MonsterSpawnerComponent &&
		MonsterSpawnerComponent->HasSpawnedMonsters();
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
	const float EntranceOffset = 200.f;

	FVector LocalLocation = FVector::ZeroVector;

	switch (Direction)
	{
	case EDoorDirection::Up:
		LocalLocation = FVector(0.f, RoomHalfExtent - EntranceOffset, 0.f);
		break;

	case EDoorDirection::Down:
		LocalLocation = FVector(0.f, -RoomHalfExtent + EntranceOffset, 0.f);
		break;

	case EDoorDirection::Left:
		LocalLocation = FVector(-RoomHalfExtent + EntranceOffset, 0.f, 0.f);
		break;

	case EDoorDirection::Right:
		LocalLocation = FVector(RoomHalfExtent - EntranceOffset, 0.f, 0.f);
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
		CachedRoomData, RoomHalfExtent, InteriorCellSize, InteriorMargin, CachedMapSeed);
	CachedInteriorLayout = Layout;

	/* 생성된 경로 데이터로 실제 path actor를 생성 */
	SpawnGuaranteedPaths(Layout);
	SpawnLargeStructureMeshes(Layout);
	if (MonsterSpawnerComponent)
	{
		MonsterSpawnerComponent->SpawnMonsters(CachedRoomData, CachedInteriorLayout, CachedMapSeed, GetActorTransform(), CachedTendency);
	}
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

	switch (Direction)
	{
	case EDoorDirection::Up:
		LocalLocation = FVector(0.f, RoomHalfExtent - DoorInset, DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, 0.f, 0.f);
		break;

	case EDoorDirection::Down:
		LocalLocation = FVector(0.f, -RoomHalfExtent + DoorInset, DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, 180.f, 0.f);
		break;

	case EDoorDirection::Left:
		LocalLocation = FVector(-RoomHalfExtent + DoorInset, 0.f, DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, -90.f, 0.f);
		break;

	case EDoorDirection::Right:
		LocalLocation = FVector(RoomHalfExtent - DoorInset, 0.f, DoorSpawnZOffset);
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

	// ObstacleBypass가 높을수록 장애물 많이 배치 (돌파형 플레이어에게 더 도전적인 환경)
	const int32 ObstacleCount = FMath::RoundToInt(
		FMath::Lerp(0.f, static_cast<float>(MaxObstaclesPerRoom), Tendency.ObstacleBypass));

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

	// 방 중앙 60% 영역에 배치 (문 주변 제외)
	const float SafeZone = RoomHalfExtent * 0.6f;
	const float MinSeparation = 400.f;

	for (int32 i = 0; i < ObstacleCount; i++)
	{
		FVector LocalPos;
		bool bFound = false;

		for (int32 Try = 0; Try < 15; Try++)
		{
			LocalPos = FVector(
				RandomStream.FRandRange(-SafeZone, SafeZone),
				RandomStream.FRandRange(-SafeZone, SafeZone),
				0.f);

			bool bTooClose = false;
			for (const TObjectPtr<AAnalysisObstacle>& Existing : SpawnedObstacles)
			{
				if (IsValid(Existing) &&
					FVector::Dist(GetActorTransform().TransformPosition(LocalPos), Existing->GetActorLocation()) < MinSeparation)
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
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		const FVector WorldPos = GetActorTransform().TransformPosition(LocalPos);
		const FRotator WorldRot = FRotator(0.f, RandomStream.FRandRange(0.f, 360.f), 0.f);

		AAnalysisObstacle* Spawned = World->SpawnActor<AAnalysisObstacle>(ObstacleClass, WorldPos, WorldRot, SpawnParams);
		if (Spawned)
		{
			SpawnedObstacles.Add(Spawned);
			UE_LOG(LogTemp, Log, TEXT("RoomActor: 장애물 스폰 (ObstacleBypass=%.2f, %d/%d)"),
				Tendency.ObstacleBypass, SpawnedObstacles.Num(), ObstacleCount);
		}
	}
}
