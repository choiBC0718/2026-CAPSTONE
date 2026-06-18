// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/RoomDecor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Map/RoomActor/RoomActor.h"
#include "Map/RoomActor/Interior/RoomDecorationSet.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomActor/Interior/RoomInteriorGrid.h"
#include "Map/RoomActor/Interior/RoomInteriorTemplateActor.h"

namespace
{
	FVector GetCellLocalCenter(const FRoomInteriorLayout& Layout, const FIntPoint& CellCoord)
	{
		const float GridWorldSizeX = Layout.GridWidth * Layout.CellSize;
		const float GridWorldSizeY = Layout.GridHeight * Layout.CellSize;
		const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, 0.f);

		return FVector(
			GridMin.X + (CellCoord.X * Layout.CellSize) + (Layout.CellSize * 0.5f),
			GridMin.Y + (CellCoord.Y * Layout.CellSize) + (Layout.CellSize * 0.5f),
			0.f);
	}

	void ApplyDecorationCollision(UStaticMeshComponent& MeshComponent, ERoomDecorCollisionType CollisionType)
	{
		switch (CollisionType)
		{
		case ERoomDecorCollisionType::Blocking:
			MeshComponent.SetCollisionProfileName(TEXT("BlockAll"));
			MeshComponent.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			MeshComponent.SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
			break;

		case ERoomDecorCollisionType::OverlapOnly:
			MeshComponent.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			MeshComponent.SetCollisionResponseToAllChannels(ECR_Overlap);
			break;

		case ERoomDecorCollisionType::VisualOnly:
		default:
			MeshComponent.SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		}
	}

	bool IsInsideFloorExclusion(
		const FRoomInteriorLayout& Layout,
		const FRoomInteriorCell& Cell,
		const FTransform& RoomTransform,
		const FTransform& TemplateTransform,
		const TArray<FRoomInteriorFloorExclusionBox>& ExclusionBoxes)
	{
		const FVector CellLocalCenter = GetCellLocalCenter(Layout, Cell.Coord);
		const FVector TemplateLocalCenter = TemplateTransform.InverseTransformPosition(RoomTransform.TransformPosition(CellLocalCenter));

		for (const FRoomInteriorFloorExclusionBox& ExclusionBox : ExclusionBoxes)
		{
			const FVector Delta = TemplateLocalCenter - ExclusionBox.Center;
			if (
				FMath::Abs(Delta.X) <= ExclusionBox.Extent.X &&
				FMath::Abs(Delta.Y) <= ExclusionBox.Extent.Y)
			{
				return true;
			}
		}

		return false;
	}

	void CollectDecorationCandidateCells(
		const FRoomInteriorLayout& Layout,
		const FTransform& RoomTransform,
		const FTransform& TemplateTransform,
		const TArray<FRoomInteriorFloorExclusionBox>& ExclusionBoxes,
		TArray<int32>& OutCandidateCellIndices)
	{
		OutCandidateCellIndices.Reset();
		OutCandidateCellIndices.Reserve(Layout.Cells.Num());

		for (int32 CellIndex = 0; CellIndex < Layout.Cells.Num(); ++CellIndex)
		{
			const FRoomInteriorCell& Cell = Layout.Cells[CellIndex];
			if (Cell.Type != ERoomInteriorCellType::Empty)
			{
				continue;
			}

			if (IsInsideFloorExclusion(Layout, Cell, RoomTransform, TemplateTransform, ExclusionBoxes))
			{
				continue;
			}

			OutCandidateCellIndices.Add(CellIndex);
		}
	}

	void ShuffleCellIndices(TArray<int32>& CellIndices, FRandomStream& RandomStream)
	{
		for (int32 Index = CellIndices.Num() - 1; Index > 0; --Index)
		{
			const int32 SwapIndex = RandomStream.RandRange(0, Index);
			CellIndices.Swap(Index, SwapIndex);
		}
	}

	TArray<FIntPoint> BuildLargeAnchorTargets(const FRoomInteriorLayout& Layout)
	{
		const int32 Left = 1;
		const int32 Bottom = 1;
		const int32 Right = FMath::Max(0, Layout.GridWidth - 2);
		const int32 Top = FMath::Max(0, Layout.GridHeight - 2);
		TArray<FIntPoint> Targets;
		Targets.Reserve(4);
		Targets.Add(FIntPoint(Left, Bottom));
		Targets.Add(FIntPoint(Right, Bottom));
		Targets.Add(FIntPoint(Left, Top));
		Targets.Add(FIntPoint(Right, Top));
		return Targets;
	}

	int32 FindNearestCandidateToAnchor(
		const FRoomInteriorLayout& Layout,
		const TArray<int32>& CandidateCellIndices,
		const TSet<int32>& UsedCellIndices,
		const FIntPoint& AnchorTarget)
	{
		int32 BestCellIndex = INDEX_NONE;
		int32 BestDistanceSquared = TNumericLimits<int32>::Max();

		for (const int32 CandidateCellIndex : CandidateCellIndices)
		{
			if (UsedCellIndices.Contains(CandidateCellIndex) || !Layout.Cells.IsValidIndex(CandidateCellIndex))
			{
				continue;
			}

			const FRoomInteriorCell& Cell = Layout.Cells[CandidateCellIndex];
			const int32 DeltaX = Cell.Coord.X - AnchorTarget.X;
			const int32 DeltaY = Cell.Coord.Y - AnchorTarget.Y;
			const int32 DistanceSquared = (DeltaX * DeltaX) + (DeltaY * DeltaY);
			if (DistanceSquared < BestDistanceSquared)
			{
				BestDistanceSquared = DistanceSquared;
				BestCellIndex = CandidateCellIndex;
			}
		}

		return BestCellIndex;
	}

	void CollectClusterCandidateCells(
		const FRoomInteriorLayout& Layout,
		const TArray<int32>& CandidateCellIndices,
		const TSet<int32>& UsedCellIndices,
		const FIntPoint& AnchorCoord,
		int32 Radius,
		TArray<int32>& OutClusterCellIndices)
	{
		OutClusterCellIndices.Reset();
		Radius = FMath::Max(0, Radius);

		for (const int32 CandidateCellIndex : CandidateCellIndices)
		{
			if (UsedCellIndices.Contains(CandidateCellIndex) || !Layout.Cells.IsValidIndex(CandidateCellIndex))
			{
				continue;
			}

			const FRoomInteriorCell& Cell = Layout.Cells[CandidateCellIndex];
			const int32 DeltaX = FMath::Abs(Cell.Coord.X - AnchorCoord.X);
			const int32 DeltaY = FMath::Abs(Cell.Coord.Y - AnchorCoord.Y);
			if (DeltaX <= Radius && DeltaY <= Radius)
			{
				OutClusterCellIndices.Add(CandidateCellIndex);
			}
		}
	}

	AActor* SpawnLargeDecorationActor(
		ARoomActor& Room,
		const FRoomInteriorLayout& Layout,
		const FRoomInteriorCell& Cell,
		const FRoomLargeDecorationEntry& Entry,
		FRandomStream& RandomStream)
	{
		const float MinScale = FMath::Min(Entry.UniformScaleRange.X, Entry.UniformScaleRange.Y);
		const float MaxScale = FMath::Max(Entry.UniformScaleRange.X, Entry.UniformScaleRange.Y);
		const float UniformScale = RandomStream.FRandRange(MinScale, MaxScale);
		const float Yaw = RandomStream.FRandRange(-Entry.YawJitterDegrees, Entry.YawJitterDegrees);
		const FVector CellLocalCenter = GetCellLocalCenter(Layout, Cell.Coord) + FVector(0.f, 0.f, Entry.ZOffset);
		const FTransform LocalTransform(FRotator(0.f, Yaw, 0.f), CellLocalCenter, FVector(UniformScale));
		const FTransform SpawnTransform = LocalTransform * Room.GetActorTransform();

		return Room.GetWorld()->SpawnActor<AActor>(Entry.ActorClass, SpawnTransform);
	}

	void MarkBlockedRadius(FRoomInteriorLayout& Layout, const FIntPoint& CenterCoord, int32 Radius)
	{
		Radius = FMath::Max(0, Radius);
		for (FRoomInteriorCell& Cell : Layout.Cells)
		{
			if (Cell.Type != ERoomInteriorCellType::Empty)
			{
				continue;
			}

			const int32 DeltaX = FMath::Abs(Cell.Coord.X - CenterCoord.X);
			const int32 DeltaY = FMath::Abs(Cell.Coord.Y - CenterCoord.Y);
			if (DeltaX <= Radius && DeltaY <= Radius)
			{
				Cell.Type = ERoomInteriorCellType::Blocked;
			}
		}
	}

	void SpawnLargeDecorations(
		ARoomActor& Room,
		FRoomInteriorLayout& Layout,
		const TArray<int32>& CandidateCellIndices,
		const URoomDecorationSet& DecorationSet,
		const FRoomTemplateDecorationRule& TemplateRule,
		TArray<TObjectPtr<AActor>>& OutSpawnedLargeDecorationActors,
		FRandomStream& RandomStream)
	{
		const int32 MinLargeDecorCount = FMath::Max(0, TemplateRule.MinLargeDecorCount);
		const int32 MaxLargeDecorCount = FMath::Max(MinLargeDecorCount, TemplateRule.MaxLargeDecorCount);
		if (MaxLargeDecorCount <= 0 || CandidateCellIndices.IsEmpty())
		{
			return;
		}

		TArray<FIntPoint> AnchorTargets = BuildLargeAnchorTargets(Layout);
		for (int32 Index = AnchorTargets.Num() - 1; Index > 0; --Index)
		{
			const int32 SwapIndex = RandomStream.RandRange(0, Index);
			AnchorTargets.Swap(Index, SwapIndex);
		}

		const int32 TargetLargeDecorCount = FMath::Min(
			AnchorTargets.Num(),
			RandomStream.RandRange(MinLargeDecorCount, MaxLargeDecorCount));

		TSet<int32> UsedCellIndices;
		for (int32 SpawnIndex = 0; SpawnIndex < TargetLargeDecorCount; ++SpawnIndex)
		{
			if (!AnchorTargets.IsValidIndex(SpawnIndex))
			{
				break;
			}

			const int32 CellIndex = FindNearestCandidateToAnchor(Layout, CandidateCellIndices, UsedCellIndices, AnchorTargets[SpawnIndex]);
			if (!Layout.Cells.IsValidIndex(CellIndex))
			{
				continue;
			}

			const FRoomLargeDecorationEntry* Entry = DecorationSet.PickLargeEntry(RandomStream);
			if (!Entry || !Entry->ActorClass)
			{
				continue;
			}

			TArray<int32> ClusterCellIndices;
			CollectClusterCandidateCells(
				Layout,
				CandidateCellIndices,
				UsedCellIndices,
				Layout.Cells[CellIndex].Coord,
				Entry->ClusterRadiusInCells,
				ClusterCellIndices);
			ShuffleCellIndices(ClusterCellIndices, RandomStream);

			const int32 MinClusterCount = FMath::Max(1, Entry->MinClusterCount);
			const int32 MaxClusterCount = FMath::Max(MinClusterCount, Entry->MaxClusterCount);
			const int32 TargetClusterCount = FMath::Min(
				ClusterCellIndices.Num(),
				RandomStream.RandRange(MinClusterCount, MaxClusterCount));

			for (int32 ClusterIndex = 0; ClusterIndex < TargetClusterCount; ++ClusterIndex)
			{
				const int32 ClusterCellIndex = ClusterCellIndices[ClusterIndex];
				if (!Layout.Cells.IsValidIndex(ClusterCellIndex))
				{
					continue;
				}

				FRoomInteriorCell& ClusterCell = Layout.Cells[ClusterCellIndex];
				AActor* DecorationActor = SpawnLargeDecorationActor(Room, Layout, ClusterCell, *Entry, RandomStream);
				if (!DecorationActor)
				{
					continue;
				}

				OutSpawnedLargeDecorationActors.Add(DecorationActor);
				UsedCellIndices.Add(ClusterCellIndex);
				MarkBlockedRadius(Layout, ClusterCell.Coord, Entry->BlockRadiusInCells);
			}
		}
	}
}

void FRoomDecor::Clear(ARoomActor& Room)
{
	for (AActor* DecorationActor : Room.SpawnedLargeDecorationActors)
	{
		if (IsValid(DecorationActor))
		{
			DecorationActor->Destroy();
		}
	}

	for (UStaticMeshComponent* MeshComponent : Room.SpawnedDecorationMeshes)
	{
		if (IsValid(MeshComponent))
		{
			MeshComponent->DestroyComponent();
		}
	}

	Room.SpawnedLargeDecorationActors.Empty();
	Room.SpawnedDecorationMeshes.Empty();
}

void FRoomDecor::SpawnDecorations(ARoomActor& Room, FRoomInteriorLayout& Layout)
{
	if (!Room.DecorationSet || !Room.SpawnedInteriorTemplateActor || !Room.Root || Layout.CellSize <= 0.f)
	{
		return;
	}

	const ARoomInteriorTemplateActor* TemplateActor = Room.SpawnedInteriorTemplateActor.Get();
	TArray<FRoomInteriorFloorExclusionBox> ExclusionBoxes;
	TemplateActor->GetAllFloorExclusionBoxes(ExclusionBoxes);

	int32 Seed = Room.CachedMapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(Room.CachedRoomData.GridPos));
	Seed = HashCombineFast(Seed, 0x269F34A7);
	FRandomStream RandomStream(Seed);

	const FTransform RoomTransform = Room.GetActorTransform();
	const FTransform TemplateTransform = TemplateActor->GetActorTransform();

	TArray<int32> CandidateCellIndices;
	CollectDecorationCandidateCells(Layout, RoomTransform, TemplateTransform, ExclusionBoxes, CandidateCellIndices);

	if (CandidateCellIndices.IsEmpty())
	{
		return;
	}

	SpawnLargeDecorations(
		Room,
		Layout,
		CandidateCellIndices,
		*Room.DecorationSet,
		Room.SelectedInteriorTemplateRule,
		Room.SpawnedLargeDecorationActors,
		RandomStream);
	CollectDecorationCandidateCells(Layout, RoomTransform, TemplateTransform, ExclusionBoxes, CandidateCellIndices);
	ShuffleCellIndices(CandidateCellIndices, RandomStream);

	const int32 MinSmallDecorCount = FMath::Max(0, Room.SelectedInteriorTemplateRule.MinSmallDecorCount);
	const int32 MaxSmallDecorCount = FMath::Max(MinSmallDecorCount, Room.SelectedInteriorTemplateRule.MaxSmallDecorCount);
	if (MaxSmallDecorCount <= 0 || CandidateCellIndices.IsEmpty())
	{
		return;
	}

	const int32 TargetDecorationCount = FMath::Min(
		CandidateCellIndices.Num(),
		RandomStream.RandRange(MinSmallDecorCount, MaxSmallDecorCount));

	for (int32 SpawnIndex = 0; SpawnIndex < TargetDecorationCount; ++SpawnIndex)
	{
		const int32 CellIndex = CandidateCellIndices[SpawnIndex];
		FRoomInteriorCell& Cell = Layout.Cells[CellIndex];
		const FRoomSmallDecorationEntry* Entry = Room.DecorationSet->PickSmallEntry(RandomStream);
		if (!Entry || !Entry->Mesh)
		{
			continue;
		}

		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(&Room);
		if (!MeshComponent)
		{
			continue;
		}

		const float UniformScale = RandomStream.FRandRange(Entry->UniformScaleRange.X, Entry->UniformScaleRange.Y);
		const float Yaw = RandomStream.FRandRange(-Entry->YawJitterDegrees, Entry->YawJitterDegrees);
		const FVector CellLocalCenter = GetCellLocalCenter(Layout, Cell.Coord);

		MeshComponent->SetupAttachment(Room.Root);
		MeshComponent->SetStaticMesh(Entry->Mesh);
		MeshComponent->SetRelativeLocation(CellLocalCenter + FVector(0.f, 0.f, Entry->ZOffset));
		MeshComponent->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
		MeshComponent->SetRelativeScale3D(FVector(UniformScale));
		MeshComponent->SetMobility(EComponentMobility::Movable);
		ApplyDecorationCollision(*MeshComponent, Entry->CollisionType);
		MeshComponent->RegisterComponent();
		Room.AddInstanceComponent(MeshComponent);

		Room.SpawnedDecorationMeshes.Add(MeshComponent);

		if (Entry->CollisionType == ERoomDecorCollisionType::Blocking)
		{
			Cell.Type = ERoomInteriorCellType::Blocked;
		}
	}
}
