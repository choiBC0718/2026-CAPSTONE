// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/RoomTemplate.h"

#include "Map/RoomActor/RoomActor.h"
#include "Map/RoomActor/Interior/RoomDecorationSet.h"
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
	Room.SelectedInteriorTemplateRule = FRoomTemplateDecorationRule();

	if (Room.CachedRoomData.RoomType != ERoomType::Normal)
	{
		return;
	}

	if (!Room.bUseInteriorTemplates || !Room.DecorationSet)
	{
		return;
	}

	int32 Seed = Room.CachedMapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(Room.CachedRoomData.GridPos));
	Seed = HashCombineFast(Seed, 0x32B9D4A1);
	FRandomStream RandomStream(Seed);

	const FRoomTemplateDecorationRule* SelectedRule = Room.DecorationSet->PickTemplateRule(Room.CachedRoomData, RandomStream);
	if (!SelectedRule || !SelectedRule->TemplateClass)
	{
		return;
	}

	Room.SelectedInteriorTemplateRule = *SelectedRule;
	Room.SelectedInteriorTemplateClass = SelectedRule->TemplateClass;
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

	const FTransform RelativeTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::OneVector);
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
