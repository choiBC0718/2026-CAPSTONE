// Fill out your copyright notice in the Description page of Project Settings.

#include "RoomActor.h"
#include "Map/RoomActor/DoorActor.h"
#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomActor/Interior/PCG/RoomPathActor.h"
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
	ClearSpawnedPathActors();

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
	ClearSpawnedPathActors();
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

void ARoomActor::ClearSpawnedPathActors()
{
	for (ARoomPathActor* PathActor : SpawnedPathActors)
	{
		if (IsValid(PathActor))
		{
			PathActor->Destroy();
		}
	}

	SpawnedPathActors.Empty();
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

void ARoomActor::GenerateAndSpawnInterior()
{
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
		CachedRoomData, RoomHalfExtent, InteriorCellSize, InteriorMargin, CachedMapSeed);

	SpawnGuaranteedPaths(Layout);
}

void ARoomActor::SpawnGuaranteedPaths(const FRoomInteriorLayout& Layout)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector PathOffset(0.f, 0.f, PathZOffset);

	for (const FRoomInteriorPath& Path : Layout.GuaranteedPaths)
	{
		if (Path.PathPoints.Num() < 2)
		{
			continue;
		}

		TArray<FVector> WorldPathPoints;
		WorldPathPoints.Reserve(Path.PathPoints.Num());

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

		ARoomPathActor* SpawnedPathActor = World->SpawnActor<ARoomPathActor>(
			ARoomPathActor::StaticClass(),
			FTransform::Identity,
			SpawnParams
		);

		if (!SpawnedPathActor)
		{
			continue;
		}

		SpawnedPathActor->InitializePath(WorldPathPoints);
		SpawnedPathActors.Add(SpawnedPathActor);
	}
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
