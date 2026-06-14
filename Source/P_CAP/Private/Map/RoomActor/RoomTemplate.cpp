// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/RoomTemplate.h"

#include "Map/RoomActor/RoomActor.h"
#include "Map/RoomActor/Interior/RoomInteriorTemplateActor.h"
#include "Engine/World.h"

void FRoomTemplate::Clear(ARoomActor& Room)
{
	if (IsValid(Room.SpawnedInteriorTemplateActor))
	{
		Room.SpawnedInteriorTemplateActor->Destroy();
	}

	Room.SpawnedInteriorTemplateActor = nullptr;
}

void FRoomTemplate::Select(ARoomActor& Room)
{
	Room.SelectedInteriorTemplateClass = nullptr;

	if (Room.CachedRoomData.RoomType != ERoomType::Normal)
	{
		return;
	}

	if (!Room.bUseInteriorTemplates || Room.InteriorTemplateClasses.IsEmpty())
	{
		return;
	}

	int32 Seed = Room.CachedMapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(Room.CachedRoomData.GridPos));
	Seed = HashCombineFast(Seed, 0x32B9D4A1);
	FRandomStream RandomStream(Seed);

	TArray<TSubclassOf<ARoomInteriorTemplateActor>> ValidTemplateClasses;
	ValidTemplateClasses.Reserve(Room.InteriorTemplateClasses.Num());
	for (const TSubclassOf<ARoomInteriorTemplateActor>& TemplateClass : Room.InteriorTemplateClasses)
	{
		if (TemplateClass)
		{
			ValidTemplateClasses.Add(TemplateClass);
		}
	}

	if (ValidTemplateClasses.IsEmpty())
	{
		return;
	}

	const int32 PickedIndex = RandomStream.RandRange(0, ValidTemplateClasses.Num() - 1);
	Room.SelectedInteriorTemplateClass = ValidTemplateClasses[PickedIndex];
}

void FRoomTemplate::Spawn(ARoomActor& Room)
{
	if (!Room.SelectedInteriorTemplateClass)
	{
		return;
	}

	if (Room.SpawnedInteriorTemplateActor)
	{
		return;
	}

	UWorld* World = Room.GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform RelativeTransform(Room.InteriorTemplateRelativeRotation, Room.InteriorTemplateRelativeLocation, FVector::OneVector);
	const FTransform SpawnTransform = RelativeTransform * Room.GetActorTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = &Room;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Room.SpawnedInteriorTemplateActor = World->SpawnActor<ARoomInteriorTemplateActor>(
		Room.SelectedInteriorTemplateClass,
		SpawnTransform,
		SpawnParams);

	if (!Room.SpawnedInteriorTemplateActor)
	{
		return;
	}

	Room.SpawnedInteriorTemplateActor->AttachToActor(&Room, FAttachmentTransformRules::KeepWorldTransform);
	Room.SpawnedInteriorTemplateActor->InitializeTemplate(Room.CachedMapSeed, Room.CachedRoomData.GridPos);
}
