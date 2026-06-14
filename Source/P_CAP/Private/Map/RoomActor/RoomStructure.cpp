// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/RoomStructure.h"

#include "DrawDebugHelpers.h"
#include "Map/RoomActor/RoomActor.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomActor/Interior/RoomInteriorPropSet.h"

void FRoomStructure::Clear(ARoomActor& Room)
{
	for (UStaticMeshComponent* MeshComponent : Room.SpawnedStructureMeshes)
	{
		if (IsValid(MeshComponent))
		{
			MeshComponent->DestroyComponent();
		}
	}

	Room.SpawnedStructureMeshes.Empty();
}

void FRoomStructure::SpawnLargeMeshes(ARoomActor& Room, const FRoomInteriorLayout& Layout)
{
	if (!Room.LargeStructurePropSet || Layout.CellSize <= 0.f)
	{
		return;
	}

	const float GridWorldSizeX = Layout.GridWidth * Layout.CellSize;
	const float GridWorldSizeY = Layout.GridHeight * Layout.CellSize;
	const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, 0.f);

	int32 Seed = Room.CachedMapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(Room.CachedRoomData.GridPos));
	FRandomStream RandomStream(Seed);

	for (const FRoomInteriorPlacedStructure& Structure : Layout.PlacedStructures)
	{
		const FRoomInteriorPropRule* Rule = Room.LargeStructurePropSet->FindRule(Structure.Category, Structure.Footprint);
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

		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(&Room);
		if (!MeshComponent)
		{
			continue;
		}

		const FVector StructureSize(Structure.Footprint.X * Layout.CellSize, Structure.Footprint.Y * Layout.CellSize, 0.f);
		FVector LocalCenter(
			GridMin.X + (Structure.Origin.X * Layout.CellSize) + (StructureSize.X * 0.5f),
			GridMin.Y + (Structure.Origin.Y * Layout.CellSize) + (StructureSize.Y * 0.5f),
			0.f);
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

		MeshComponent->SetupAttachment(Room.Root);
		MeshComponent->SetStaticMesh(SelectedVariant->Mesh);
		MeshComponent->SetRelativeLocation(LocalCenter);
		MeshComponent->SetRelativeRotation(FRotator(0.f, BaseYaw + YawJitter, 0.f));
		MeshComponent->SetRelativeScale3D(FVector(UniformScale));
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->RegisterComponent();
		Room.AddInstanceComponent(MeshComponent);

		Room.SpawnedStructureMeshes.Add(MeshComponent);
	}
}

void FRoomStructure::DrawDebugCells(const ARoomActor& Room, const FRoomInteriorLayout& Layout)
{
	if (!Room.bDrawInteriorCellDebug)
	{
		return;
	}

	UWorld* World = Room.GetWorld();
	if (!World || Layout.CellSize <= 0.f)
	{
		return;
	}

	const float CellHalfSize = Layout.CellSize * 0.5f;
	const float GridWorldSizeX = Layout.GridWidth * Layout.CellSize;
	const float GridWorldSizeY = Layout.GridHeight * Layout.CellSize;
	const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, Room.PathZOffset + 60.f);

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
			GridMin.Z);
		const FVector WorldCenter = Room.GetActorTransform().TransformPosition(LocalCenter);

		DrawDebugBox(
			World,
			WorldCenter,
			FVector(CellHalfSize * 0.45f, CellHalfSize * 0.45f, 35.f),
			Room.GetActorQuat(),
			DebugColor,
			true,
			-1.f,
			1,
			6.f);

		DrawDebugSolidBox(
			World,
			WorldCenter,
			FVector(CellHalfSize * 0.42f, CellHalfSize * 0.42f, 25.f),
			Room.GetActorQuat(),
			FColor(DebugColor.R, DebugColor.G, DebugColor.B, 72),
			true,
			-1.f,
			1);
	}
}
