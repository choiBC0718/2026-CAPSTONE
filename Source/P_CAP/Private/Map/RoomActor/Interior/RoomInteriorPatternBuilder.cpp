// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorPatternBuilder.h"

#include "Map/RoomActor/Interior/RoomInteriorGrid.h"
#include "Map/RoomActor/Interior/RoomInteriorValidator.h"

void FRoomInteriorPatternBuilder::PlaceLargeStructurePattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream)
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

void FRoomInteriorPatternBuilder::PlaceSecondaryStructures(
	FRoomInteriorLayout& OutLayout,
	const FRoomData& RoomData,
	float RoomHalfExtent,
	FRandomStream& RandomStream)
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
			if (FRoomInteriorGrid::GetCellType(OutLayout, Coord) != ERoomInteriorCellType::Empty)
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

		if (FRoomInteriorValidator::TryPlaceStructureWithValidation(
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

void FRoomInteriorPatternBuilder::PlaceDeadEndPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream)
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

void FRoomInteriorPatternBuilder::PlaceStraightPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream)
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

void FRoomInteriorPatternBuilder::PlaceCornerPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream)
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

void FRoomInteriorPatternBuilder::PlaceHubPattern(FRoomInteriorLayout& OutLayout, FRandomStream& RandomStream)
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

void FRoomInteriorPatternBuilder::PlaceCandidateStructures(
	FRoomInteriorLayout& OutLayout,
	const TArray<TArray<FRoomInteriorPlacedStructure>>& CandidateGroups,
	FRandomStream& RandomStream)
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
			if (!FRoomInteriorValidator::TryPlaceStructure(CandidateLayout, Structure.Origin, Structure.Footprint))
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
