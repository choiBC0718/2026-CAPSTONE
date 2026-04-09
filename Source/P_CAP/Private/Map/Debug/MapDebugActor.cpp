// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/Debug/MapDebugActor.h"
#include "Map/Debug/MapManager.h"
#include "Map/MapLayout.h"
#include "Map/RoomData.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AMapDebugActor::AMapDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMapDebugActor::RefreshDebugDraw()
{
	if (!MapManager)
	{
		MapManager = Cast<AMapManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AMapManager::StaticClass())
		);
	}

	if (!MapManager)
	{
		return;
	}

	FlushPersistentDebugLines(GetWorld());
	DebugDrawMap(MapManager->GetCurrentLayout());
}

void AMapDebugActor::BeginPlay()
{
	Super::BeginPlay();

	MapManager = Cast<AMapManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMapManager::StaticClass())
	);

	if (!MapManager)
	{
		return;
	}

	DebugDrawMap(MapManager->GetCurrentLayout());
}

void AMapDebugActor::DebugDrawMap(const FMapLayout& Layout)
{
	const float TileSize = 300.f;

	for (const TPair<FIntPoint, FRoomData>& Pair : Layout.Rooms)
	{
		const FRoomData& Room = Pair.Value;

		const FVector WorldPos(
			Room.GridPos.X * TileSize,
			Room.GridPos.Y * TileSize,
			0.f
		);

		FColor Color = FColor::White;

		switch (Room.RoomType)
		{
		case ERoomType::Start:
			Color = FColor::Green;
			break;

		case ERoomType::Boss:
			Color = FColor::Red;
			break;

		case ERoomType::Reward:
			Color = FColor::Yellow;
			break;

		case ERoomType::Shop:
			Color = FColor::Blue;
			break;

		case ERoomType::Event:
			Color = FColor::White;
			break;

		case ERoomType::Normal:
		default:
			Color = FColor::White;
			break;
		}

		DrawDebugBox(GetWorld(), WorldPos, FVector(100.f, 100.f, 50.f), Color, true);
	}
}