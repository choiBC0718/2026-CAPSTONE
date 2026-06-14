// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/RoomFloor.h"

#include "Map/RoomActor/RoomActor.h"
#include "Map/RoomActor/Interior/RoomInteriorTemplateActor.h"
#include "Components/StaticMeshComponent.h"

void FRoomFloor::ClearVisualTiles(ARoomActor& Room)
{
	for (UStaticMeshComponent* MeshComponent : Room.SpawnedVisualFloorTileMeshes)
	{
		if (IsValid(MeshComponent))
		{
			MeshComponent->DestroyComponent();
		}
	}

	Room.SpawnedVisualFloorTileMeshes.Empty();
	ApplyBaseVisibility(Room);
}

void FRoomFloor::SpawnVisualTiles(ARoomActor& Room)
{
	if (!Room.bGenerateVisualFloorTiles || !Room.VisualFloorTileMesh || !Room.Root)
	{
		return;
	}

	const float RoomSize = Room.GetEffectiveRoomHalfExtent() * 2.f;
	const float CellSize = FMath::Max(Room.GetEffectiveInteriorCellSize(), 1.f);
	const float TargetTileSize = CellSize * FMath::Max(1, Room.VisualFloorTileSizeInCells);
	const int32 TileCountX = FMath::Max(1, FMath::CeilToInt(RoomSize / TargetTileSize));
	const int32 TileCountY = FMath::Max(1, FMath::CeilToInt(RoomSize / TargetTileSize));
	const float TiledRoomSizeX = TileCountX * TargetTileSize;
	const float TiledRoomSizeY = TileCountY * TargetTileSize;
	const FVector GridMin(-TiledRoomSizeX * 0.5f, -TiledRoomSizeY * 0.5f, Room.VisualFloorTileZOffset);

	const FBoxSphereBounds MeshBounds = Room.VisualFloorTileMesh->GetBounds();
	const float MeshSizeX = FMath::Max(MeshBounds.BoxExtent.X * 2.f, 1.f);
	const float MeshSizeY = FMath::Max(MeshBounds.BoxExtent.Y * 2.f, 1.f);
	const FVector TileScale(
		(TargetTileSize / MeshSizeX) * Room.VisualFloorTileScaleMultiplier,
		(TargetTileSize / MeshSizeY) * Room.VisualFloorTileScaleMultiplier,
		Room.VisualFloorTileScaleMultiplier);

	int32 Seed = Room.CachedMapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(Room.CachedRoomData.GridPos));
	Seed = HashCombineFast(Seed, 0x6A3D20F1);
	FRandomStream RandomStream(Seed);

	Room.SpawnedVisualFloorTileMeshes.Reserve(TileCountX * TileCountY);
	const FVector TileLocalExtent(TargetTileSize * 0.5f, TargetTileSize * 0.5f, 100.f);
	for (int32 Y = 0; Y < TileCountY; ++Y)
	{
		for (int32 X = 0; X < TileCountX; ++X)
		{
			UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(&Room);
			if (!MeshComponent)
			{
				continue;
			}

			const FVector LocalCenter(
				GridMin.X + (X * TargetTileSize) + (TargetTileSize * 0.5f),
				GridMin.Y + (Y * TargetTileSize) + (TargetTileSize * 0.5f),
				GridMin.Z);
			if (ShouldSkipVisualTileAtLocalBounds(Room, LocalCenter, TileLocalExtent))
			{
				continue;
			}

			const float Yaw = Room.bRandomizeVisualFloorTileRotation
				? 90.f * RandomStream.RandRange(0, 3)
				: 0.f;
			const FRotator TileRotation(0.f, Yaw, 0.f);
			const FVector BoundsCenterOffset = TileRotation.RotateVector(FVector(
				MeshBounds.Origin.X * TileScale.X,
				MeshBounds.Origin.Y * TileScale.Y,
				MeshBounds.Origin.Z * TileScale.Z));

			MeshComponent->SetupAttachment(Room.Root);
			MeshComponent->SetStaticMesh(Room.VisualFloorTileMesh);
			MeshComponent->SetRelativeLocation(LocalCenter - BoundsCenterOffset);
			MeshComponent->SetRelativeRotation(TileRotation);
			MeshComponent->SetRelativeScale3D(TileScale);
			if (Room.bUseVisualFloorTileCollision)
			{
				MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
			}
			else
			{
				MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			MeshComponent->SetMobility(EComponentMobility::Movable);
			MeshComponent->RegisterComponent();
			Room.AddInstanceComponent(MeshComponent);

			Room.SpawnedVisualFloorTileMeshes.Add(MeshComponent);
		}
	}
}

bool FRoomFloor::ShouldSkipVisualTileAtLocalBounds(const ARoomActor& Room, const FVector& LocalCenter, const FVector& LocalExtent)
{
	if (!Room.SelectedInteriorTemplateClass)
	{
		return false;
	}

	const ARoomInteriorTemplateActor* TemplateSource = Room.SpawnedInteriorTemplateActor
		? Room.SpawnedInteriorTemplateActor.Get()
		: Room.SelectedInteriorTemplateClass->GetDefaultObject<ARoomInteriorTemplateActor>();
	if (!TemplateSource)
	{
		return false;
	}

	const FTransform TemplateTransform = Room.SpawnedInteriorTemplateActor
		? Room.SpawnedInteriorTemplateActor->GetActorTransform()
		: FTransform(Room.InteriorTemplateRelativeRotation, Room.InteriorTemplateRelativeLocation, FVector::OneVector) * Room.GetActorTransform();
	const FVector WorldCenter = Room.GetActorTransform().TransformPosition(LocalCenter);
	const FVector WorldExtent = Room.GetActorTransform().TransformVectorNoScale(LocalExtent).GetAbs();
	const FVector TemplateLocalCenter = TemplateTransform.InverseTransformPosition(WorldCenter);
	const FVector TemplateLocalExtent = TemplateTransform.InverseTransformVectorNoScale(WorldExtent).GetAbs();

	TArray<FRoomInteriorFloorExclusionBox> ExclusionBoxes;
	TemplateSource->GetAllFloorExclusionBoxes(ExclusionBoxes);
	for (const FRoomInteriorFloorExclusionBox& ExclusionBox : ExclusionBoxes)
	{
		const FVector EffectiveExclusionExtent(
			FMath::Max(0.f, ExclusionBox.Extent.X + Room.VisualFloorExclusionPadding),
			FMath::Max(0.f, ExclusionBox.Extent.Y + Room.VisualFloorExclusionPadding),
			FMath::Max(0.f, ExclusionBox.Extent.Z + Room.VisualFloorExclusionPadding));
		const FVector EffectiveTileExtent = Room.VisualFloorExclusionMode == ERoomVisualFloorExclusionMode::TileBoundsOverlap
			? TemplateLocalExtent
			: FVector::ZeroVector;
		const FVector Delta = TemplateLocalCenter - ExclusionBox.Center;
		if (FMath::Abs(Delta.X) <= (EffectiveTileExtent.X + EffectiveExclusionExtent.X) &&
			FMath::Abs(Delta.Y) <= (EffectiveTileExtent.Y + EffectiveExclusionExtent.Y))
		{
			return true;
		}
	}

	return false;
}

void FRoomFloor::ApplyBaseVisibility(ARoomActor& Room)
{
	if (!Room.FloorMesh)
	{
		return;
	}

	if (!Room.bHasInitialFloorMeshCollisionEnabled)
	{
		Room.InitialFloorMeshCollisionEnabled = Room.FloorMesh->GetCollisionEnabled();
		Room.bHasInitialFloorMeshCollisionEnabled = true;
	}

	const bool bUseVisualFloor = Room.bGenerateVisualFloorTiles && Room.VisualFloorTileMesh && Room.bHideBaseFloorMeshWhenUsingVisualTiles;
	Room.FloorMesh->SetHiddenInGame(bUseVisualFloor);
	Room.FloorMesh->SetVisibility(!bUseVisualFloor, true);
	Room.FloorMesh->SetCollisionEnabled(bUseVisualFloor ? ECollisionEnabled::NoCollision : Room.InitialFloorMeshCollisionEnabled.GetValue());
}
