// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomActor.h"
#include "Map/RoomActor/DoorActor.h"
#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

ARoomActor::ARoomActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	FloorMesh->SetupAttachment(Root);
}

void ARoomActor::InitializeRoom(const FRoomData& InRoomData, int32 InMapSeed)
{
	CachedRoomData = InRoomData;
	CachedMapSeed = InMapSeed;

	ClearSpawnedDoors();
	ClearSpawnedInteriorMeshes();

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
	ClearSpawnedDoors();
	ClearSpawnedInteriorMeshes();
	Super::Destroyed();
}

void ARoomActor::ClearSpawnedDoors()
{
	for (ADoorActor* Door : SpawnedDoors)
	{
		if (IsValid(Door))
		{
			Door->Destroy();
		}
	}

	SpawnedDoors.Empty();
}

void ARoomActor::SpawnConnectedDoors()
{
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
	SpawnedDoor->InitializeDoor(CachedRoomData.GridPos, TargetRoomPos, Direction);

	SpawnedDoors.Add(SpawnedDoor);
}

void ARoomActor::ClearSpawnedInteriorMeshes()
{
	for (UStaticMeshComponent* MeshComp : SpawnedInteriorMeshes)
	{
		if (IsValid(MeshComp))
		{
			MeshComp->DestroyComponent();
		}
	}

	SpawnedInteriorMeshes.Empty();
}

void ARoomActor::GenerateAndSpawnInterior()
{
	if (CachedRoomData.RoomType != ERoomType::Normal)
	{
		return;
	}

	if (!ObstacleMesh)
	{
		return;
	}

	/* 생성기 준비 */
	if (!InteriorGenerator)
	{
		InteriorGenerator = NewObject<URoomInteriorGenerator>(this);
	}

	if (!InteriorGenerator)
	{
		return;
	}

	const FRoomInteriorLayout Layout = InteriorGenerator->GenerateInteriorLayout(
		CachedRoomData,	RoomHalfExtent,	InteriorCellSize, InteriorMargin, CachedMapSeed);

	for (const FRoomInteriorCell& Cell : Layout.Cells)
	{
		if (Cell.CellType != ERoomInteriorCellType::Obstacle)
		{
			continue;
		}

		SpawnObstacleMeshAtLocalPosition(Cell.LocalPosition);
	}
}

void ARoomActor::SpawnObstacleMeshAtLocalPosition(const FVector& LocalPosition)
{
	if (!ObstacleMesh)
	{
		return;
	}

	UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(this);
	if (!MeshComp)
	{
		return;
	}

	MeshComp->RegisterComponent();
	MeshComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	MeshComp->SetStaticMesh(ObstacleMesh);
	MeshComp->SetRelativeLocation(LocalPosition + FVector(0.f, 0.f, ObstacleHeightOffset));
	MeshComp->SetRelativeRotation(FRotator::ZeroRotator);
	MeshComp->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	SpawnedInteriorMeshes.Add(MeshComp);
}

FTransform ARoomActor::GetDoorTransform(EDoorDirection Direction) const
{
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

	return FTransform(WorldRotation, WorldLocation, FVector::OneVector);
}

FIntPoint ARoomActor::GetNeighborGridPos(EDoorDirection Direction) const
{
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
