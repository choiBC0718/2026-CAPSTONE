// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Containers/Queue.h"

FRoomInteriorLayout URoomInteriorGenerator::GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const
{
	/* 최종 결과를 담을 내부 레이아웃 */
	FRoomInteriorLayout Layout;
	(void)Margin;
	(void)MapSeed;

	/* 먼저 보장 경로를 계산한 뒤 이를 기준으로 셀 레이아웃을 만든다 */
	BuildGuaranteedPaths(Layout, RoomData, RoomHalfExtent);
	InitializeCellGrid(Layout, RoomHalfExtent, CellSize);
	MarkDoorReservedCells(Layout, RoomData, RoomHalfExtent);
	FRandomStream RandomStream = MakeRoomRandomStream(RoomData, MapSeed);
	PlaceLargeStructurePattern(Layout, RoomData, RandomStream);
	PlaceSecondaryStructures(Layout, RoomData, RoomHalfExtent, RandomStream);

	if (!AreAllDoorsReachable(Layout, RoomData, RoomHalfExtent))
	{
		for (FRoomInteriorCell& Cell : Layout.Cells)
		{
			if (Cell.Type == ERoomInteriorCellType::Blocked)
			{
				Cell.Type = ERoomInteriorCellType::Empty;
			}
		}

		Layout.PlacedStructures.Empty();
	}

	return Layout;
}

FRandomStream URoomInteriorGenerator::MakeRoomRandomStream(const FRoomData& RoomData, int32 MapSeed) const
{
	int32 Seed = MapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(RoomData.GridPos));
	Seed = HashCombineFast(Seed, static_cast<int32>(RoomData.RoomType));
	Seed = HashCombineFast(Seed, RoomData.GetConnectionCount());
	return FRandomStream(Seed);
}

void URoomInteriorGenerator::InitializeCellGrid(FRoomInteriorLayout& OutLayout, float RoomHalfExtent, float CellSize) const
{
	const float SafeCellSize = FMath::Max(CellSize, 1.f);
	const float RoomSize = RoomHalfExtent * 2.f;
	const int32 GridSize = FMath::Max(FMath::FloorToInt(RoomSize / SafeCellSize), 1);

	OutLayout.GridWidth = GridSize;
	OutLayout.GridHeight = GridSize;
	OutLayout.CellSize = SafeCellSize;
	OutLayout.Cells.Empty();
	OutLayout.Cells.Reserve(GridSize * GridSize);

	for (int32 Y = 0; Y < GridSize; ++Y)
	{
		for (int32 X = 0; X < GridSize; ++X)
		{
			FRoomInteriorCell& NewCell = OutLayout.Cells.AddDefaulted_GetRef();
			NewCell.Coord = FIntPoint(X, Y);
			NewCell.Type = ERoomInteriorCellType::Empty;
		}
	}
}

void URoomInteriorGenerator::MarkDoorReservedCells(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent) const
{
	const TArray<FVector> DoorAnchors = GetDoorAnchors(RoomData, RoomHalfExtent);

	for (const FVector& DoorAnchor : DoorAnchors)
	{
		const FIntPoint DoorCell = LocalPointToCell(OutLayout, DoorAnchor, RoomHalfExtent);
		SetCellType(OutLayout, DoorCell, ERoomInteriorCellType::ReservedDoor);

		const FVector DirectionToCenter = (-DoorAnchor).GetSafeNormal2D();
		for (int32 Step = 1; Step <= 2; ++Step)
		{
			const FVector OffsetPoint = DoorAnchor + (DirectionToCenter * OutLayout.CellSize * Step);
			SetCellType(OutLayout, LocalPointToCell(OutLayout, OffsetPoint, RoomHalfExtent), ERoomInteriorCellType::ReservedDoor);
		}
	}
}

void URoomInteriorGenerator::MarkGuaranteedPathCells(FRoomInteriorLayout& OutLayout, float RoomHalfExtent) const
{
	for (const FRoomInteriorPath& Path : OutLayout.GuaranteedPaths)
	{
		if (Path.PathPoints.Num() < 2)
		{
			continue;
		}

		const float ReserveRadius = FMath::Max(Path.CorridorWidth * 0.5f, OutLayout.CellSize);
		const int32 SampleCountPerSegment = 8;

		for (int32 Index = 0; Index + 1 < Path.PathPoints.Num(); ++Index)
		{
			const FVector Start = Path.PathPoints[Index];
			const FVector End = Path.PathPoints[Index + 1];

			for (int32 SampleIndex = 0; SampleIndex <= SampleCountPerSegment; ++SampleIndex)
			{
				const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(SampleCountPerSegment);
				const FVector SamplePoint = FMath::Lerp(Start, End, Alpha);
				const FIntPoint CenterCell = LocalPointToCell(OutLayout, SamplePoint, RoomHalfExtent);
				const int32 RadiusInCells = FMath::Max(FMath::CeilToInt(ReserveRadius / OutLayout.CellSize), 1);

				for (int32 OffsetY = -RadiusInCells; OffsetY <= RadiusInCells; ++OffsetY)
				{
					for (int32 OffsetX = -RadiusInCells; OffsetX <= RadiusInCells; ++OffsetX)
					{
						const FIntPoint TestCell = CenterCell + FIntPoint(OffsetX, OffsetY);
						if (!IsValidCell(OutLayout, TestCell))
						{
							continue;
						}

						SetCellType(OutLayout, TestCell, ERoomInteriorCellType::ReservedPath);
					}
				}
			}
		}
	}
}

void URoomInteriorGenerator::PlaceLargeStructurePattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream) const
{
	const int32 ConnectionCount = RoomData.GetConnectionCount();

	if (ConnectionCount <= 0)
	{
		return;
	}

	if (ConnectionCount == 1)
	{
		PlaceDeadEndPattern(OutLayout, RoomData, RandomStream);
		return;
	}

	const bool bVerticalStraight = RoomData.bConnectedUp && RoomData.bConnectedDown;
	const bool bHorizontalStraight = RoomData.bConnectedLeft && RoomData.bConnectedRight;
	if (ConnectionCount == 2 && (bVerticalStraight || bHorizontalStraight))
	{
		PlaceStraightPattern(OutLayout, RoomData, RandomStream);
		return;
	}

	if (ConnectionCount == 2)
	{
		PlaceCornerPattern(OutLayout, RoomData, RandomStream);
		return;
	}

	PlaceHubPattern(OutLayout, RandomStream);
}

void URoomInteriorGenerator::PlaceSecondaryStructures(
	FRoomInteriorLayout& OutLayout,
	const FRoomData& RoomData,
	float RoomHalfExtent,
	FRandomStream& RandomStream) const
{
	struct FSecondaryCandidate
	{
		FIntPoint Origin = FIntPoint::ZeroValue;
		FIntPoint Footprint = FIntPoint(1, 1);
		ERoomInteriorStructureCategory Category = ERoomInteriorStructureCategory::Generic;
		int32 Priority = 0;
	};

	TArray<FSecondaryCandidate> Candidates;

	for (int32 Y = 0; Y < OutLayout.GridHeight; ++Y)
	{
		for (int32 X = 0; X < OutLayout.GridWidth; ++X)
		{
			const FIntPoint Coord(X, Y);
			if (GetCellType(OutLayout, Coord) != ERoomInteriorCellType::Empty)
			{
				continue;
			}

			const bool bNearLeftWall = X <= 1;
			const bool bNearRightWall = X >= OutLayout.GridWidth - 2;
			const bool bNearBottomWall = Y <= 1;
			const bool bNearTopWall = Y >= OutLayout.GridHeight - 2;
			const bool bNearAnyWall = bNearLeftWall || bNearRightWall || bNearBottomWall || bNearTopWall;
			const bool bNearCorner = (bNearLeftWall || bNearRightWall) && (bNearBottomWall || bNearTopWall);
			const bool bNearOuterBand = X <= 2 || X >= OutLayout.GridWidth - 3 || Y <= 2 || Y >= OutLayout.GridHeight - 3;
			const bool bNearCenterBand =
				X >= (OutLayout.GridWidth / 2) - 1 && X <= (OutLayout.GridWidth / 2)
				&& Y >= (OutLayout.GridHeight / 2) - 1 && Y <= (OutLayout.GridHeight / 2);

			if (bNearAnyWall)
			{
				FSecondaryCandidate& NewCandidate = Candidates.AddDefaulted_GetRef();
				NewCandidate.Origin = Coord;
				NewCandidate.Footprint = (bNearTopWall || bNearBottomWall) ? FIntPoint(2, 1) : FIntPoint(1, 2);
				NewCandidate.Category = bNearCorner ? ERoomInteriorStructureCategory::Corner : ERoomInteriorStructureCategory::Side;
				NewCandidate.Priority = bNearCorner ? 4 : 3;
			}

			if (bNearOuterBand && !bNearCenterBand)
			{
				FSecondaryCandidate& SmallCandidate = Candidates.AddDefaulted_GetRef();
				SmallCandidate.Origin = Coord;
				SmallCandidate.Footprint = FIntPoint(1, 1);
				SmallCandidate.Category = bNearCorner ? ERoomInteriorStructureCategory::Corner : ERoomInteriorStructureCategory::Generic;
				SmallCandidate.Priority = bNearCorner ? 2 : 1;
			}
		}
	}

	for (int32 Index = Candidates.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = RandomStream.RandRange(0, Index);
		Candidates.Swap(Index, SwapIndex);
	}

	Candidates.StableSort([](const FSecondaryCandidate& A, const FSecondaryCandidate& B)
	{
		return A.Priority > B.Priority;
	});

	const int32 ConnectionCount = RoomData.GetConnectionCount();
	const int32 MaxSecondaryLarge = ConnectionCount <= 1 ? 2 : (ConnectionCount == 2 ? 2 : 3);
	const int32 MaxSecondarySmall = ConnectionCount <= 1 ? 1 : 1;
	int32 PlacedLargeCount = 0;
	int32 PlacedSmallCount = 0;

	for (const FSecondaryCandidate& Candidate : Candidates)
	{
		const bool bIsSmall = Candidate.Footprint == FIntPoint(1, 1);
		if (bIsSmall)
		{
			if (PlacedSmallCount >= MaxSecondarySmall)
			{
				continue;
			}
		}
		else if (PlacedLargeCount >= MaxSecondaryLarge)
		{
			continue;
		}

		if (TryPlaceStructureWithValidation(
			OutLayout,
			RoomData,
			RoomHalfExtent,
			Candidate.Origin,
			Candidate.Footprint,
			Candidate.Category))
		{
			if (bIsSmall)
			{
				++PlacedSmallCount;
			}
			else
			{
				++PlacedLargeCount;
			}
		}
	}
}

void URoomInteriorGenerator::PlaceDeadEndPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream) const
{
	const int32 CenterX = OutLayout.GridWidth / 2;
	const int32 CenterY = OutLayout.GridHeight / 2;
	TArray<TArray<FRoomInteriorPlacedStructure>> CandidateGroups;

	if (RoomData.bConnectedUp)
	{
		CandidateGroups = {
			{{FIntPoint(CenterX - 1, 1), FIntPoint(2, 2)}},
			{{FIntPoint(1, 1), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, 1), FIntPoint(2, 2)}},
			{{FIntPoint(CenterX - 1, 2), FIntPoint(2, 2)}, {FIntPoint(1, 1), FIntPoint(1, 2)}}
		};
		PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
		return;
	}

	if (RoomData.bConnectedDown)
	{
		CandidateGroups = {
			{{FIntPoint(CenterX - 1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(CenterX - 1, OutLayout.GridHeight - 4), FIntPoint(2, 2)}, {FIntPoint(OutLayout.GridWidth - 2, OutLayout.GridHeight - 4), FIntPoint(1, 2)}}
		};
		PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
		return;
	}

	if (RoomData.bConnectedLeft)
	{
		CandidateGroups = {
			{{FIntPoint(OutLayout.GridWidth - 3, CenterY - 1), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, 1), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 4, CenterY - 1), FIntPoint(2, 2)}, {FIntPoint(OutLayout.GridWidth - 4, 1), FIntPoint(2, 1)}}
		};
		PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
		return;
	}

	if (RoomData.bConnectedRight)
	{
		CandidateGroups = {
			{{FIntPoint(1, CenterY - 1), FIntPoint(2, 2)}},
			{{FIntPoint(1, 1), FIntPoint(2, 2)}},
			{{FIntPoint(1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(2, CenterY - 1), FIntPoint(2, 2)}, {FIntPoint(2, OutLayout.GridHeight - 2), FIntPoint(2, 1)}}
		};
		PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
	}
}

void URoomInteriorGenerator::PlaceStraightPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream) const
{
	const bool bVerticalStraight = RoomData.bConnectedUp && RoomData.bConnectedDown;
	const int32 CenterX = OutLayout.GridWidth / 2;
	const int32 CenterY = OutLayout.GridHeight / 2;
	TArray<TArray<FRoomInteriorPlacedStructure>> CandidateGroups;

	if (bVerticalStraight)
	{
		CandidateGroups = {
			{{FIntPoint(1, 1), FIntPoint(2, 2)}, {FIntPoint(OutLayout.GridWidth - 3, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}, {FIntPoint(OutLayout.GridWidth - 3, 1), FIntPoint(2, 2)}},
			{{FIntPoint(1, CenterY - 1), FIntPoint(2, 2)}, {FIntPoint(OutLayout.GridWidth - 3, CenterY - 1), FIntPoint(2, 2)}},
			{{FIntPoint(1, CenterY - 1), FIntPoint(1, 2)}, {FIntPoint(OutLayout.GridWidth - 2, CenterY - 1), FIntPoint(1, 2)}, {FIntPoint(CenterX - 1, 1), FIntPoint(2, 1)}}
		};
		PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
		return;
	}

	CandidateGroups = {
		{{FIntPoint(1, 1), FIntPoint(2, 2)}, {FIntPoint(OutLayout.GridWidth - 3, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
		{{FIntPoint(OutLayout.GridWidth - 3, 1), FIntPoint(2, 2)}, {FIntPoint(1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
		{{FIntPoint(CenterX - 1, 1), FIntPoint(2, 2)}, {FIntPoint(CenterX - 1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
		{{FIntPoint(CenterX - 1, 1), FIntPoint(2, 1)}, {FIntPoint(CenterX - 1, OutLayout.GridHeight - 2), FIntPoint(2, 1)}, {FIntPoint(1, CenterY - 1), FIntPoint(1, 2)}}
	};
	PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
}

void URoomInteriorGenerator::PlaceCornerPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream) const
{
	const bool bTop = RoomData.bConnectedUp;
	const bool bBottom = RoomData.bConnectedDown;
	const bool bLeft = RoomData.bConnectedLeft;
	const bool bRight = RoomData.bConnectedRight;
	TArray<TArray<FRoomInteriorPlacedStructure>> CandidateGroups;

	if (bTop && bRight)
	{
		CandidateGroups = {
			{{FIntPoint(1, 1), FIntPoint(2, 2)}},
			{{FIntPoint(1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, 1), FIntPoint(2, 2)}},
			{{FIntPoint(1, 1), FIntPoint(2, 1)}, {FIntPoint(1, 2), FIntPoint(1, 2)}}
		};
		PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
		return;
	}

	if (bTop && bLeft)
	{
		CandidateGroups = {
			{{FIntPoint(OutLayout.GridWidth - 3, 1), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(1, 1), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, 1), FIntPoint(2, 1)}, {FIntPoint(OutLayout.GridWidth - 2, 2), FIntPoint(1, 2)}}
		};
		PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
		return;
	}

	if (bBottom && bRight)
	{
		CandidateGroups = {
			{{FIntPoint(1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(1, 1), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(1, OutLayout.GridHeight - 3), FIntPoint(2, 1)}, {FIntPoint(1, OutLayout.GridHeight - 4), FIntPoint(1, 2)}}
		};
		PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
		return;
	}

	if (bBottom && bLeft)
	{
		CandidateGroups = {
			{{FIntPoint(OutLayout.GridWidth - 3, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, 1), FIntPoint(2, 2)}},
			{{FIntPoint(1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}},
			{{FIntPoint(OutLayout.GridWidth - 3, OutLayout.GridHeight - 3), FIntPoint(2, 1)}, {FIntPoint(OutLayout.GridWidth - 2, OutLayout.GridHeight - 4), FIntPoint(1, 2)}}
		};
		PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
	}
}

void URoomInteriorGenerator::PlaceHubPattern(FRoomInteriorLayout& OutLayout, FRandomStream& RandomStream) const
{
	const int32 MidX = OutLayout.GridWidth / 2;
	const int32 MidY = OutLayout.GridHeight / 2;
	TArray<TArray<FRoomInteriorPlacedStructure>> CandidateGroups = {
		{
			{FIntPoint(1, 1), FIntPoint(2, 2)},
			{FIntPoint(OutLayout.GridWidth - 3, 1), FIntPoint(2, 2)},
			{FIntPoint(1, OutLayout.GridHeight - 3), FIntPoint(2, 2)},
			{FIntPoint(OutLayout.GridWidth - 3, OutLayout.GridHeight - 3), FIntPoint(2, 2)}
		},
		{
			{FIntPoint(1, MidY - 1), FIntPoint(2, 2)},
			{FIntPoint(OutLayout.GridWidth - 3, MidY - 1), FIntPoint(2, 2)},
			{FIntPoint(MidX - 1, 1), FIntPoint(2, 2)},
			{FIntPoint(MidX - 1, OutLayout.GridHeight - 3), FIntPoint(2, 2)}
		},
		{
			{FIntPoint(MidX - 1, 1), FIntPoint(2, 1)},
			{FIntPoint(MidX - 1, OutLayout.GridHeight - 2), FIntPoint(2, 1)},
			{FIntPoint(1, MidY - 1), FIntPoint(1, 2)},
			{FIntPoint(OutLayout.GridWidth - 2, MidY - 1), FIntPoint(1, 2)}
		}
	};
	PlaceCandidateStructures(OutLayout, CandidateGroups, RandomStream);
}

void URoomInteriorGenerator::PlaceCandidateStructures(
	FRoomInteriorLayout& OutLayout,
	const TArray<TArray<FRoomInteriorPlacedStructure>>& CandidateGroups,
	FRandomStream& RandomStream) const
{
	if (CandidateGroups.IsEmpty())
	{
		return;
	}

	TArray<int32> CandidateIndices;
	CandidateIndices.Reserve(CandidateGroups.Num());
	for (int32 Index = 0; Index < CandidateGroups.Num(); ++Index)
	{
		CandidateIndices.Add(Index);
	}

	for (int32 Index = CandidateIndices.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = RandomStream.RandRange(0, Index);
		CandidateIndices.Swap(Index, SwapIndex);
	}

	for (const int32 CandidateIndex : CandidateIndices)
	{
		const TArray<FRoomInteriorPlacedStructure>& Candidate = CandidateGroups[CandidateIndex];
		FRoomInteriorLayout CandidateLayout = OutLayout;
		bool bAllPlaced = true;

		for (const FRoomInteriorPlacedStructure& Structure : Candidate)
		{
			if (!TryPlaceStructure(CandidateLayout, Structure.Origin, Structure.Footprint))
			{
				bAllPlaced = false;
				break;
			}
		}

		if (bAllPlaced)
		{
			OutLayout = CandidateLayout;
			return;
		}
	}
}

bool URoomInteriorGenerator::TryPlaceStructure(FRoomInteriorLayout& OutLayout, const FIntPoint& Origin, const FIntPoint& Footprint) const
{
	constexpr int32 RequiredGapInCells = 1;
	if (!HasRequiredStructureSpacing(OutLayout, Origin, Footprint, RequiredGapInCells))
	{
		return false;
	}

	for (int32 Y = 0; Y < Footprint.Y; ++Y)
	{
		for (int32 X = 0; X < Footprint.X; ++X)
		{
			const FIntPoint Coord = Origin + FIntPoint(X, Y);
			if (!IsValidCell(OutLayout, Coord))
			{
				return false;
			}

			const ERoomInteriorCellType CellType = GetCellType(OutLayout, Coord);
			if (CellType != ERoomInteriorCellType::Empty)
			{
				return false;
			}
		}
	}

	for (int32 Y = 0; Y < Footprint.Y; ++Y)
	{
		for (int32 X = 0; X < Footprint.X; ++X)
		{
			SetCellType(OutLayout, Origin + FIntPoint(X, Y), ERoomInteriorCellType::Blocked);
		}
	}

	FRoomInteriorPlacedStructure& NewStructure = OutLayout.PlacedStructures.AddDefaulted_GetRef();
	NewStructure.Origin = Origin;
	NewStructure.Footprint = Footprint;
	return true;
}

bool URoomInteriorGenerator::HasRequiredStructureSpacing(
	const FRoomInteriorLayout& Layout,
	const FIntPoint& Origin,
	const FIntPoint& Footprint,
	int32 RequiredGapInCells) const
{
	const FIntPoint MinCoord = Origin - FIntPoint(RequiredGapInCells, RequiredGapInCells);
	const FIntPoint MaxCoord = Origin + Footprint - FIntPoint(1, 1) + FIntPoint(RequiredGapInCells, RequiredGapInCells);

	for (int32 Y = MinCoord.Y; Y <= MaxCoord.Y; ++Y)
	{
		for (int32 X = MinCoord.X; X <= MaxCoord.X; ++X)
		{
			const FIntPoint Coord(X, Y);
			if (!IsValidCell(Layout, Coord))
			{
				continue;
			}

			if (GetCellType(Layout, Coord) == ERoomInteriorCellType::Blocked)
			{
				return false;
			}
		}
	}

	return true;
}

bool URoomInteriorGenerator::TryPlaceStructureWithValidation(
	FRoomInteriorLayout& OutLayout,
	const FRoomData& RoomData,
	float RoomHalfExtent,
	const FIntPoint& Origin,
	const FIntPoint& Footprint,
	ERoomInteriorStructureCategory Category) const
{
	FRoomInteriorLayout CandidateLayout = OutLayout;
	if (!TryPlaceStructure(CandidateLayout, Origin, Footprint))
	{
		return false;
	}

	if (!AreAllDoorsReachable(CandidateLayout, RoomData, RoomHalfExtent))
	{
		return false;
	}

	OutLayout = CandidateLayout;
	if (OutLayout.PlacedStructures.Num() > 0)
	{
		OutLayout.PlacedStructures.Last().Category = Category;
	}

	return true;
}

bool URoomInteriorGenerator::AreAllDoorsReachable(const FRoomInteriorLayout& Layout, const FRoomData& RoomData, float RoomHalfExtent) const
{
	TArray<FIntPoint> DoorCells;
	const TArray<FVector> DoorAnchors = GetDoorAnchors(RoomData, RoomHalfExtent);
	DoorCells.Reserve(DoorAnchors.Num());

	for (const FVector& DoorAnchor : DoorAnchors)
	{
		const FIntPoint DoorCell = LocalPointToCell(Layout, DoorAnchor, RoomHalfExtent);
		if (IsValidCell(Layout, DoorCell))
		{
			DoorCells.Add(DoorCell);
		}
	}

	if (DoorCells.Num() <= 1)
	{
		return true;
	}

	TSet<FIntPoint> Visited;
	TQueue<FIntPoint> Queue;
	Queue.Enqueue(DoorCells[0]);
	Visited.Add(DoorCells[0]);

	while (!Queue.IsEmpty())
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		static const FIntPoint Directions[] =
		{
			FIntPoint(1, 0),
			FIntPoint(-1, 0),
			FIntPoint(0, 1),
			FIntPoint(0, -1)
		};

		for (const FIntPoint& Direction : Directions)
		{
			const FIntPoint Next = Current + Direction;
			if (!IsValidCell(Layout, Next) || Visited.Contains(Next))
			{
				continue;
			}

			const ERoomInteriorCellType CellType = GetCellType(Layout, Next);
			if (CellType == ERoomInteriorCellType::Blocked)
			{
				continue;
			}

			Visited.Add(Next);
			Queue.Enqueue(Next);
		}
	}

	for (const FIntPoint& DoorCell : DoorCells)
	{
		if (!Visited.Contains(DoorCell))
		{
			return false;
		}
	}

	return true;
}

FIntPoint URoomInteriorGenerator::LocalPointToCell(const FRoomInteriorLayout& Layout, const FVector& LocalPoint, float RoomHalfExtent) const
{
	const float ShiftedX = (LocalPoint.X + RoomHalfExtent) / Layout.CellSize;
	const float ShiftedY = (LocalPoint.Y + RoomHalfExtent) / Layout.CellSize;

	const int32 CellX = FMath::Clamp(FMath::FloorToInt(ShiftedX), 0, Layout.GridWidth - 1);
	const int32 CellY = FMath::Clamp(FMath::FloorToInt(ShiftedY), 0, Layout.GridHeight - 1);
	return FIntPoint(CellX, CellY);
}

bool URoomInteriorGenerator::IsValidCell(const FRoomInteriorLayout& Layout, const FIntPoint& Coord) const
{
	return Coord.X >= 0 && Coord.X < Layout.GridWidth
		&& Coord.Y >= 0 && Coord.Y < Layout.GridHeight;
}

int32 URoomInteriorGenerator::GetCellIndex(const FRoomInteriorLayout& Layout, const FIntPoint& Coord) const
{
	return Coord.Y * Layout.GridWidth + Coord.X;
}

ERoomInteriorCellType URoomInteriorGenerator::GetCellType(const FRoomInteriorLayout& Layout, const FIntPoint& Coord) const
{
	if (!IsValidCell(Layout, Coord))
	{
		return ERoomInteriorCellType::Blocked;
	}

	return Layout.Cells[GetCellIndex(Layout, Coord)].Type;
}

void URoomInteriorGenerator::SetCellType(FRoomInteriorLayout& OutLayout, const FIntPoint& Coord, ERoomInteriorCellType NewType) const
{
	if (!IsValidCell(OutLayout, Coord))
	{
		return;
	}

	FRoomInteriorCell& Cell = OutLayout.Cells[GetCellIndex(OutLayout, Coord)];

	/* 문 앞 예약은 다른 예약보다 우선순위를 가진다 */
	if (Cell.Type == ERoomInteriorCellType::ReservedDoor && NewType != ERoomInteriorCellType::ReservedDoor)
	{
		return;
	}

	Cell.Type = NewType;
}

void URoomInteriorGenerator::BuildGuaranteedPaths(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent) const
{
	/* 현재 방에서 실제로 연결된 문들의 로컬 좌표를 수집 */
	const TArray<FVector> DoorAnchors = GetDoorAnchors(RoomData, RoomHalfExtent);
	if (DoorAnchors.IsEmpty())
	{
		return;
	}

	/* 경로 허브는 항상 방 중심을 사용 */
	const FVector HubPoint = GetHubPoint();
	/* 나중에 PCG에서 경로 주변 금지 폭으로 쓸 수 있도록 함께 저장 */
	const float CorridorWidth = FMath::Max(RoomHalfExtent * 0.2f, 100.f);

	/* 입구만 있는 dead-end 방은
	   1) 문에서 중심으로 들어오는 경로 1개
	   2) 중심 기준 원형 경로 1개
	   를 함께 생성 */
	if (DoorAnchors.Num() == 1)
	{
		FRoomInteriorPath EntryPath;
		EntryPath.CorridorWidth = CorridorWidth;
		EntryPath.PathPoints.Add(DoorAnchors[0]);
		EntryPath.PathPoints.Add(HubPoint);
		OutLayout.GuaranteedPaths.Add(EntryPath);
		OutLayout.GuaranteedPaths.Add(BuildDeadEndLoopPath(RoomHalfExtent, CorridorWidth));
		return;
	}

	/* 문이 2개면 방 중심을 반드시 경유하도록 꺾인 경로 1개를 만든다
	   - 서로 마주보는 문이면 사실상 직선
	   - 서로 직교하는 문이면 ㄴ/ㄱ 형태 */
	if (DoorAnchors.Num() == 2)
	{
		FRoomInteriorPath Path;
		Path.CorridorWidth = CorridorWidth;
		Path.PathPoints.Add(DoorAnchors[0]);
		Path.PathPoints.Add(HubPoint);
		Path.PathPoints.Add(DoorAnchors[1]);
		OutLayout.GuaranteedPaths.Add(Path);
		return;
	}

	/* 문이 3개 이상이면 각 문이 방 중심으로 모이는 허브형 구조를 만든다 */
	for (const FVector& DoorAnchor : DoorAnchors)
	{
		FRoomInteriorPath Path;
		Path.CorridorWidth = CorridorWidth;
		Path.PathPoints.Add(DoorAnchor);
		Path.PathPoints.Add(HubPoint);
		OutLayout.GuaranteedPaths.Add(Path);
	}
}

FRoomInteriorPath URoomInteriorGenerator::BuildDeadEndLoopPath(float RoomHalfExtent, float CorridorWidth) const
{
	FRoomInteriorPath LoopPath;
	LoopPath.CorridorWidth = CorridorWidth;
	LoopPath.bClosedLoop = true;

	const float LoopRadius = FMath::Max(RoomHalfExtent * 0.2f, 450.f);
	const int32 SegmentCount = 16;
	LoopPath.PathPoints.Reserve(SegmentCount);

	for (int32 Index = 0; Index < SegmentCount; ++Index)
	{
		const float AngleDeg = (360.f / SegmentCount) * Index;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);

		LoopPath.PathPoints.Add(FVector(
			FMath::Cos(AngleRad) * LoopRadius,
			FMath::Sin(AngleRad) * LoopRadius,
			0.f
		));
	}

	return LoopPath;
}

TArray<FVector> URoomInteriorGenerator::GetDoorAnchors(const FRoomData& RoomData, float RoomHalfExtent) const
{
	TArray<FVector> DoorAnchors;

	/* 방의 상단 문 로컬 위치 */
	if (RoomData.bConnectedUp)
	{
		DoorAnchors.Add(FVector(0.f, RoomHalfExtent, 0.f));
	}

	/* 방의 하단 문 로컬 위치 */
	if (RoomData.bConnectedDown)
	{
		DoorAnchors.Add(FVector(0.f, -RoomHalfExtent, 0.f));
	}

	/* 방의 좌측 문 로컬 위치 */
	if (RoomData.bConnectedLeft)
	{
		DoorAnchors.Add(FVector(-RoomHalfExtent, 0.f, 0.f));
	}

	/* 방의 우측 문 로컬 위치 */
	if (RoomData.bConnectedRight)
	{
		DoorAnchors.Add(FVector(RoomHalfExtent, 0.f, 0.f));
	}

	return DoorAnchors;
}

FVector URoomInteriorGenerator::GetHubPoint() const
{
	/* 허브는 항상 방 중심 */
	return FVector::ZeroVector;
}
