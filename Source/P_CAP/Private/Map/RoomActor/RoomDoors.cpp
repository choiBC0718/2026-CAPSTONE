// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/RoomDoors.h"

#include "Map/RoomActor/RoomActor.h"
#include "Map/RoomActor/DoorActor.h"
#include "Engine/World.h"

void FRoomDoors::Clear(ARoomActor& Room)
{
	for (ADoorActor* Door : Room.SpawnedDoors)
	{
		if (IsValid(Door))
		{
			Door->Destroy();
		}
	}

	Room.SpawnedDoors.Empty();
}

void FRoomDoors::SpawnConnected(ARoomActor& Room)
{
	if (Room.CachedRoomData.bConnectedUp)
	{
		Spawn(Room, EDoorDirection::Up);
	}

	if (Room.CachedRoomData.bConnectedDown)
	{
		Spawn(Room, EDoorDirection::Down);
	}

	if (Room.CachedRoomData.bConnectedLeft)
	{
		Spawn(Room, EDoorDirection::Left);
	}

	if (Room.CachedRoomData.bConnectedRight)
	{
		Spawn(Room, EDoorDirection::Right);
	}
}

void FRoomDoors::Spawn(ARoomActor& Room, EDoorDirection Direction)
{
	if (!Room.DoorActorClass)
	{
		return;
	}

	UWorld* World = Room.GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = &Room;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADoorActor* SpawnedDoor = World->SpawnActor<ADoorActor>(
		Room.DoorActorClass,
		GetTransform(Room, Direction),
		SpawnParams);

	if (!SpawnedDoor)
	{
		return;
	}

	SpawnedDoor->InitializeDoor(Room.CachedRoomData.GridPos, GetNeighborGridPos(Room, Direction), Direction);
	Room.SpawnedDoors.Add(SpawnedDoor);
}

void FRoomDoors::SetPortalEnabled(ARoomActor& Room, bool bEnabled)
{
	for (ADoorActor* Door : Room.SpawnedDoors)
	{
		if (IsValid(Door))
		{
			Door->SetPortalEnabled(bEnabled);
		}
	}
}

FTransform FRoomDoors::GetTransform(const ARoomActor& Room, EDoorDirection Direction)
{
	FVector LocalLocation = FVector::ZeroVector;
	FRotator LocalRotation = FRotator::ZeroRotator;
	const float RoomHalfExtentValue = Room.GetEffectiveRoomHalfExtent();
	const float DoorInsetValue = Room.GetEffectiveDoorInset();

	switch (Direction)
	{
	case EDoorDirection::Up:
		LocalLocation = FVector(0.f, RoomHalfExtentValue - DoorInsetValue, Room.DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, 0.f, 0.f);
		break;

	case EDoorDirection::Down:
		LocalLocation = FVector(0.f, -RoomHalfExtentValue + DoorInsetValue, Room.DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, 180.f, 0.f);
		break;

	case EDoorDirection::Left:
		LocalLocation = FVector(-RoomHalfExtentValue + DoorInsetValue, 0.f, Room.DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, -90.f, 0.f);
		break;

	case EDoorDirection::Right:
		LocalLocation = FVector(RoomHalfExtentValue - DoorInsetValue, 0.f, Room.DoorSpawnZOffset);
		LocalRotation = FRotator(0.f, 90.f, 0.f);
		break;

	default:
		break;
	}

	const FVector WorldLocation = Room.GetActorTransform().TransformPosition(LocalLocation);
	const FQuat WorldRotation = Room.GetActorQuat() * LocalRotation.Quaternion();
	return FTransform(WorldRotation, WorldLocation, FVector::OneVector);
}

FIntPoint FRoomDoors::GetNeighborGridPos(const ARoomActor& Room, EDoorDirection Direction)
{
	switch (Direction)
	{
	case EDoorDirection::Up:
		return Room.CachedRoomData.GridPos + FIntPoint(0, 1);

	case EDoorDirection::Down:
		return Room.CachedRoomData.GridPos + FIntPoint(0, -1);

	case EDoorDirection::Left:
		return Room.CachedRoomData.GridPos + FIntPoint(-1, 0);

	case EDoorDirection::Right:
		return Room.CachedRoomData.GridPos + FIntPoint(1, 0);

	default:
		return Room.CachedRoomData.GridPos;
	}
}
