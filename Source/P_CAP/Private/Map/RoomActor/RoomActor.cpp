// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomActor.h"
#include "Map/RoomActor/DoorActor.h"
#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomActor/Interior/RoomInteriorPropSet.h"
#include "Map/RoomActor/Interior/PCG/RoomPathActor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ARoomActor::ARoomActor()
{
	PrimaryActorTick.bCanEverTick = false;

	/* 방의 기준이 되는 루트 */
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	/* 바닥 메시 */
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(Root);
}

void ARoomActor::InitializeRoom(const FRoomData& InRoomData, int32 InMapSeed)
{
	/* 현재 방 정보와 맵 시드를 캐싱 */
	CachedRoomData = InRoomData;
	CachedMapSeed = InMapSeed;

	/* 재초기화 상황을 대비해 기존 문/경로 액터를 정리 */
	ClearSpawnedDoors();
	ClearSpawnedPathActors();
	ClearSpawnedStructureMeshes();

	/* 방 정보 기준으로 문과 경로를 다시 생성 */
	SpawnConnectedDoors();
	GenerateAndSpawnInterior();
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
