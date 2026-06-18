// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/RoomFence.h"

#include "Map/RoomActor/RoomActor.h"
#include "Components/StaticMeshComponent.h"

void FRoomFence::Clear(ARoomActor& Room)
{
	for (UStaticMeshComponent* MeshComponent : Room.SpawnedEdgeFenceMeshes)
	{
		if (IsValid(MeshComponent))
		{
			MeshComponent->DestroyComponent();
		}
	}

	Room.SpawnedEdgeFenceMeshes.Empty();
}

void FRoomFence::Spawn(ARoomActor& Room)
{
	if (!Room.bGenerateEdgeFences || !Room.EdgeFenceMesh || !Room.Root)
	{
		return;
	}

	const float HalfExtent = Room.GetEffectiveRoomHalfExtent();
	const float InsetHalfExtent = FMath::Max(0.f, HalfExtent - Room.EdgeFenceInset);
	const float EdgeLength = InsetHalfExtent * 2.f;
	if (EdgeLength <= 0.f)
	{
		return;
	}

	const FVector TopLeft(-InsetHalfExtent, InsetHalfExtent, Room.EdgeFenceZOffset);
	const FVector TopRight(InsetHalfExtent, InsetHalfExtent, Room.EdgeFenceZOffset);
	const FVector BottomLeft(-InsetHalfExtent, -InsetHalfExtent, Room.EdgeFenceZOffset);
	const FVector BottomRight(InsetHalfExtent, -InsetHalfExtent, Room.EdgeFenceZOffset);

	SpawnLine(Room, TopLeft, FVector::ForwardVector, EdgeLength, 0.f, Room.CachedRoomData.bConnectedUp);
	SpawnLine(Room, BottomRight, FVector::BackwardVector, EdgeLength, 180.f, Room.CachedRoomData.bConnectedDown);
	SpawnLine(Room, BottomLeft, FVector::RightVector, EdgeLength, 90.f, Room.CachedRoomData.bConnectedLeft);
	SpawnLine(Room, TopRight, FVector::LeftVector, EdgeLength, -90.f, Room.CachedRoomData.bConnectedRight);
}

void FRoomFence::SpawnLine(ARoomActor& Room, const FVector& EdgeStart, const FVector& EdgeDirection, float EdgeLength, float Yaw, bool bHasDoorGap)
{
	const float CellSize = FMath::Max(Room.GetEffectiveInteriorCellSize(), 1.f);
	const float TargetSegmentLength = CellSize * FMath::Max(1, Room.EdgeFenceSegmentSizeInCells);
	const int32 SegmentCount = FMath::Max(1, FMath::FloorToInt(EdgeLength / TargetSegmentLength));
	const float SegmentSpacing = EdgeLength / SegmentCount;

	const FBoxSphereBounds MeshBounds = Room.EdgeFenceMesh->GetBounds();
	const float MeshLengthX = FMath::Max(MeshBounds.BoxExtent.X * 2.f, 1.f);
	const FVector FenceScale(
		(SegmentSpacing / MeshLengthX) * Room.EdgeFenceScaleMultiplier,
		Room.EdgeFenceScaleMultiplier,
		Room.EdgeFenceScaleMultiplier);

	const FRotator FenceRotation(0.f, Yaw, 0.f);
	for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
	{
		const float DistanceAlongEdge = (SegmentIndex + 0.5f) * SegmentSpacing;
		if (bHasDoorGap && IsInDoorGap(Room, DistanceAlongEdge, EdgeLength))
		{
			continue;
		}

		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(&Room);
		if (!MeshComponent)
		{
			continue;
		}

		const FVector LocalCenter = EdgeStart + EdgeDirection.GetSafeNormal() * DistanceAlongEdge;
		const FVector BoundsCenterOffset = FenceRotation.RotateVector(FVector(
			MeshBounds.Origin.X * FenceScale.X,
			MeshBounds.Origin.Y * FenceScale.Y,
			MeshBounds.Origin.Z * FenceScale.Z));

		MeshComponent->SetupAttachment(Room.Root);
		MeshComponent->SetStaticMesh(Room.EdgeFenceMesh);
		MeshComponent->SetRelativeLocation(LocalCenter - BoundsCenterOffset);
		MeshComponent->SetRelativeRotation(FenceRotation);
		MeshComponent->SetRelativeScale3D(FenceScale);
		MeshComponent->SetCollisionProfileName(Room.bEnableEdgeFenceCollision ? TEXT("BlockAll") : TEXT("NoCollision"));
		MeshComponent->SetCollisionEnabled(Room.bEnableEdgeFenceCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->RegisterComponent();
		Room.AddInstanceComponent(MeshComponent);

		Room.SpawnedEdgeFenceMeshes.Add(MeshComponent);
	}

	if (bHasDoorGap)
	{
		SpawnDoorSideBlocks(Room, EdgeStart, EdgeDirection, EdgeLength, Yaw);
	}
}

void FRoomFence::SpawnDoorSideBlocks(ARoomActor& Room, const FVector& EdgeStart, const FVector& EdgeDirection, float EdgeLength, float Yaw)
{
	if (!Room.DoorSideBlockMesh)
	{
		return;
	}

	const float HalfGap = Room.EdgeFenceDoorGapWidth * 0.5f;
	const float EdgeCenter = EdgeLength * 0.5f;
	const float LeftDistance = FMath::Clamp(EdgeCenter - HalfGap, 0.f, EdgeLength);
	const float RightDistance = FMath::Clamp(EdgeCenter + HalfGap, 0.f, EdgeLength);
	const FRotator BlockRotation(0.f, Yaw, 0.f);
	const FBoxSphereBounds MeshBounds = Room.DoorSideBlockMesh->GetBounds();
	const FVector BlockScale(Room.DoorSideBlockScaleMultiplier);
	const FVector BoundsCenterOffset = BlockRotation.RotateVector(FVector(
		MeshBounds.Origin.X * BlockScale.X,
		MeshBounds.Origin.Y * BlockScale.Y,
		MeshBounds.Origin.Z * BlockScale.Z));

	const float Distances[] = { LeftDistance, RightDistance };
	for (const float DistanceAlongEdge : Distances)
	{
		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(&Room);
		if (!MeshComponent)
		{
			continue;
		}

		FVector LocalCenter = EdgeStart + EdgeDirection.GetSafeNormal() * DistanceAlongEdge;
		LocalCenter.Z = Room.DoorSideBlockZOffset;

		MeshComponent->SetupAttachment(Room.Root);
		MeshComponent->SetStaticMesh(Room.DoorSideBlockMesh);
		MeshComponent->SetRelativeLocation(LocalCenter - BoundsCenterOffset);
		MeshComponent->SetRelativeRotation(BlockRotation);
		MeshComponent->SetRelativeScale3D(BlockScale);
		MeshComponent->SetCollisionProfileName(Room.bEnableEdgeFenceCollision ? TEXT("BlockAll") : TEXT("NoCollision"));
		MeshComponent->SetCollisionEnabled(Room.bEnableEdgeFenceCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		MeshComponent->SetMobility(EComponentMobility::Movable);
		MeshComponent->RegisterComponent();
		Room.AddInstanceComponent(MeshComponent);

		Room.SpawnedEdgeFenceMeshes.Add(MeshComponent);
	}
}

bool FRoomFence::IsInDoorGap(const ARoomActor& Room, float DistanceAlongEdge, float EdgeLength)
{
	const float HalfGap = Room.EdgeFenceDoorGapWidth * 0.5f;
	const float EdgeCenter = EdgeLength * 0.5f;
	return FMath::Abs(DistanceAlongEdge - EdgeCenter) <= HalfGap;
}
