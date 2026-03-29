// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/MapGenerator.h"

FMapLayout UMapGenerator::GenerateMap(const FMapGenerationConfig& InConfig)
{
	/* 새로 만든 맵 결과를 저장할 변수
	   (거리 정보, 연결 정보, 시작방, 보스방, 방 개수, 시드 등) */
	FMapLayout NewLayout;
	NewLayout.UsedConfig = InConfig;
	NewLayout.UsedSeed = InConfig.Seed;

	/* 시드를 기반으로 랜덤 생성기를 만드는 부분 */
	FRandomStream RandomStream(InConfig.Seed);

	CreateStartRoom(NewLayout);
	ExpandRooms(NewLayout, RandomStream);
	UpdateRoomConnections(NewLayout);
	CalculateDistanceFromStart(NewLayout);
	AssignBossRoom(NewLayout);

	return NewLayout;
}

void UMapGenerator::CreateStartRoom(FMapLayout& OutLayout)
{
	/* 시작방 위치를 (0,0)으로 설정 */
	const FIntPoint StartPos = FIntPoint::ZeroValue;

	/* RoomData 시작방 데이터 생성 및 기록 */
	FRoomData StartRoom(StartPos);
	StartRoom.RoomType = ERoomType::Start;
	StartRoom.DistanceFromStart = 0;

	/* OutLayout은 NewLayout을 참조로 받아서 같은 객체를 직접 수정
	 (실제로는 같은 값을 수정하는거 헷갈려서 적어요) */
	OutLayout.StartRoomPos = StartPos;
	OutLayout.Rooms.Add(StartPos, StartRoom);
}

void UMapGenerator::ExpandRooms(FMapLayout& OutLayout, FRandomStream& RandomStream)
{
	/* 생성할 목표 방 개수를 가져옴 */
	const int32 TargetRoomCount = OutLayout.UsedConfig.RoomCount;

	/* 현재 생성된 방 개수가 목표보다 적으면 반복 */
	while (OutLayout.GetRoomCount() < TargetRoomCount)
	{
		/* 현재 생성된 모든 방의 좌표 목록을 가져옴 */
		TArray<FIntPoint> ExistingRoomPositions;
		OutLayout.Rooms.GetKeys(ExistingRoomPositions);

		if (ExistingRoomPositions.IsEmpty())
		{
			break;
		}

		TArray<FRoomCandidate> Candidates;
		TSet<FIntPoint> AddedPositions;

		/* 현재 존재하는 방들 각각을 기준으로 */
		for (const FIntPoint& BaseRoomPos : ExistingRoomPositions)
		{
			/* 선택한 기준 방 주변의 비어 있는 상하좌우 좌표들을 구함 */
			const TArray<FIntPoint> AvailablePositions = GetAvailableNeighborPositions(OutLayout, BaseRoomPos);
			if (AvailablePositions.IsEmpty())
			{
				continue;
			}

			/* 상하좌우 빈 칸 후보들을 하나씩 검사 */
			for (const FIntPoint& CandidatePos : AvailablePositions)
			{
				if (AddedPositions.Contains(CandidatePos))
				{
					continue;
				}
				
				const float Weight = CalculateCandidateWeight(OutLayout, BaseRoomPos, CandidatePos);
				if (Weight <= 0.0f)
				{
					continue;
				}
				
				FRoomCandidate Candidate;
				Candidate.BaseRoomPos = BaseRoomPos;
				Candidate.NewRoomPos = CandidatePos;
				Candidate.Weight = Weight;

				Candidates.Add(Candidate);
				AddedPositions.Add(CandidatePos);
			}
		}

		if (Candidates.IsEmpty())
		{
			break;
		}
		
		const int32 PickedIndex = PickWeightedCandidateIndex(Candidates, RandomStream);
		if (PickedIndex == INDEX_NONE)
		{
			break;
		}

		/* 선택된 후보 위치를 새 방 위치로 정함 */
		const FIntPoint NewRoomPos = Candidates[PickedIndex].NewRoomPos;

		/* RoomData를 생성 및 등록 */
		FRoomData NewRoom(NewRoomPos);
		OutLayout.Rooms.Add(NewRoomPos, NewRoom);
	}
}

void UMapGenerator::UpdateRoomConnections(FMapLayout& OutLayout)
{
	/* 현재 맵에 있는 모든 방의 좌표 목록을 가져옴 */
	TArray<FIntPoint> RoomPositions;
	OutLayout.Rooms.GetKeys(RoomPositions);

	/* 모든 방 좌표를 하나씩 검사 */
	for (const FIntPoint& RoomPos : RoomPositions)
	{
		FRoomData* Room = OutLayout.FindRoom(RoomPos);
		if (!Room)
		{
			continue;
		}

		/* 해당 방향 좌표에 방이 존재하면 true, 없으면 false를 연결 정보에 넣는 코드 */
		Room->bConnectedUp    = OutLayout.HasRoom(RoomPos + FIntPoint(0, 1));
		Room->bConnectedDown  = OutLayout.HasRoom(RoomPos + FIntPoint(0, -1));
		Room->bConnectedLeft  = OutLayout.HasRoom(RoomPos + FIntPoint(-1, 0));
		Room->bConnectedRight = OutLayout.HasRoom(RoomPos + FIntPoint(1, 0));
	}
}

void UMapGenerator::CalculateDistanceFromStart(FMapLayout& OutLayout)
{
	/* 모든 방의 거리값을 먼저 -1로 초기화 */
	for (TPair<FIntPoint, FRoomData>& Pair : OutLayout.Rooms)
	{
		Pair.Value.DistanceFromStart = -1;
	}
	
    /* BFS에 사용할 큐 생성 및 시작방 좌표 큐 등록 */
	TQueue<FIntPoint> Queue;
	Queue.Enqueue(OutLayout.StartRoomPos);

	/* 시작방의 거리를 0으로 설정 */
	if (FRoomData* StartRoom = OutLayout.FindRoom(OutLayout.StartRoomPos))
	{
		StartRoom->DistanceFromStart = 0;
	}

	/* 큐에 처리할 방이 남아 있는 동안 계속 반복 */
	while (!Queue.IsEmpty())
	{
		/* 큐에서 현재 처리할 방 하나를 꺼냄 */
		FIntPoint CurrentPos;
		Queue.Dequeue(CurrentPos);

		/* 현재 좌표에 해당하는 방 데이터를 찾고 없으면 건너뜀 */
		FRoomData* CurrentRoom = OutLayout.FindRoom(CurrentPos);
		if (!CurrentRoom)
		{
			continue;
		}

		/* 현재 방에서 한 칸 이동한 이웃 방의 거리 계산 */
		const int32 NextDistance = CurrentRoom->DistanceFromStart + 1;
		/* 현재 방의 상하좌우 좌표 4개를 가져옴 */
		const TArray<FIntPoint> NeighborPositions = GetNeighborPositions(CurrentPos);

		/* 상하좌우 이웃 좌표를 하나씩 검사 */
		for (const FIntPoint& NeighborPos : NeighborPositions)
		{
			/* 그 방향에 실제 방이 있는지 확인 */
			FRoomData* NeighborRoom = OutLayout.FindRoom(NeighborPos);
			if (!NeighborRoom)
			{
				continue;
			}

			/* 이미 방문해서 거리 계산한 방이면 건너뜀 (초기 설정한 -1인 방만 처리) */
			if (NeighborRoom->DistanceFromStart != -1)
			{
				continue;
			}

			/* 이웃 방의 거리값 설정 */
			NeighborRoom->DistanceFromStart = NextDistance;
			Queue.Enqueue(NeighborPos);
		}
	}
}

void UMapGenerator::AssignBossRoom(FMapLayout& OutLayout)
{
	/* 지금까지 찾은 가장 먼 거리값을 저장하는 변수 초기화 */
	int32 MaxDistance = -1;
	FIntPoint BossPos = OutLayout.StartRoomPos;

	/* 맵에 있는 모든 방을 하나씩 검사 */
	for (const TPair<FIntPoint, FRoomData>& Pair : OutLayout.Rooms)
	{
		const FRoomData& Room = Pair.Value;

		if (Room.GridPos == OutLayout.StartRoomPos)
		{
			continue;
		}

		/* 현재 방이 지금까지 본 방들보다 더 멀리 있으면 보스 후보를 갱신 */
		if (Room.DistanceFromStart > MaxDistance)
		{
			MaxDistance = Room.DistanceFromStart;
			BossPos = Room.GridPos;
		}
	}

	/* 보스방 지정 */
	if (FRoomData* BossRoom = OutLayout.FindRoom(BossPos))
	{
		BossRoom->RoomType = ERoomType::Boss;
	}

	OutLayout.BossRoomPos = BossPos;
}

TArray<FIntPoint> UMapGenerator::GetNeighborPositions(const FIntPoint& InPos) const
{
	TArray<FIntPoint> Result;
	Result.Add(InPos + FIntPoint(0, 1));
	Result.Add(InPos + FIntPoint(0, -1));
	Result.Add(InPos + FIntPoint(-1, 0));
	Result.Add(InPos + FIntPoint(1, 0));
	return Result;
}

TArray<FIntPoint> UMapGenerator::GetAvailableNeighborPositions(const FMapLayout& InLayout, const FIntPoint& InPos) const
{
	TArray<FIntPoint> Result;
	const TArray<FIntPoint> NeighborPositions = GetNeighborPositions(InPos);

	for (const FIntPoint& NeighborPos : NeighborPositions)
	{
		if (!InLayout.HasRoom(NeighborPos))
		{
			Result.Add(NeighborPos);
		}
	}

	return Result;
}

int32 UMapGenerator::CountAdjacentRooms(const FMapLayout& InLayout, const FIntPoint& InPos) const
{
	int32 Count = 0;
	const TArray<FIntPoint> NeighborPositions = GetNeighborPositions(InPos);

	for (const FIntPoint& NeighborPos : NeighborPositions)
	{
		if (InLayout.HasRoom(NeighborPos))
		{
			Count++;
		}
	}

	return Count;
}

bool UMapGenerator::WouldCreate2x2Block(const FMapLayout& InLayout, const FIntPoint& InPos) const
{
	auto HasRoomOrNew = [&](const FIntPoint& Pos) -> bool
	{
		return Pos == InPos || InLayout.HasRoom(Pos);
	};

	// InPos를 포함하는 2x2 네 방향 검사
	if (HasRoomOrNew(InPos) &&
		HasRoomOrNew(InPos + FIntPoint(-1, 0)) &&
		HasRoomOrNew(InPos + FIntPoint(0, -1)) &&
		HasRoomOrNew(InPos + FIntPoint(-1, -1)))
	{
		return true;
	}

	if (HasRoomOrNew(InPos) &&
		HasRoomOrNew(InPos + FIntPoint(1, 0)) &&
		HasRoomOrNew(InPos + FIntPoint(0, -1)) &&
		HasRoomOrNew(InPos + FIntPoint(1, -1)))
	{
		return true;
	}

	if (HasRoomOrNew(InPos) &&
		HasRoomOrNew(InPos + FIntPoint(-1, 0)) &&
		HasRoomOrNew(InPos + FIntPoint(0, 1)) &&
		HasRoomOrNew(InPos + FIntPoint(-1, 1)))
	{
		return true;
	}

	if (HasRoomOrNew(InPos) &&
		HasRoomOrNew(InPos + FIntPoint(1, 0)) &&
		HasRoomOrNew(InPos + FIntPoint(0, 1)) &&
		HasRoomOrNew(InPos + FIntPoint(1, 1)))
	{
		return true;
	}

	return false;
}

float UMapGenerator::CalculateCandidateWeight(const FMapLayout& InLayout, const FIntPoint& BaseRoomPos,
	const FIntPoint& CandidatePos) const
{
	float Weight = 10.0f;

	// 2x2 블록 억제
	if (WouldCreate2x2Block(InLayout, CandidatePos))
	{
		return 0.0f;
	}

	// 후보 칸 주변의 방 개수에 따라 조정
	const int32 AdjacentCount = CountAdjacentRooms(InLayout, CandidatePos);

	if (AdjacentCount == 1)
	{
		Weight += 20.0f;
	}
	else if (AdjacentCount == 2)
	{
		Weight -= 10.0f;
	}
	else if (AdjacentCount >= 3)
	{
		Weight -= 50.0f;
	}

	// leaf 방에서 확장 보너스
	const FRoomData* BaseRoom = InLayout.FindRoom(BaseRoomPos);
	if (BaseRoom)
	{
		const int32 BaseAdjacentCount = CountAdjacentRooms(InLayout, BaseRoomPos);

		if (BaseAdjacentCount == 1)
		{
			Weight += 18.0f;
		}
		else if (BaseAdjacentCount >= 3)
		{
			Weight -= 10.0f;
		}
	}

	return FMath::Max(0.0f, Weight);
}

int32 UMapGenerator::PickWeightedCandidateIndex(const TArray<FRoomCandidate>& Candidates,
	FRandomStream& RandomStream) const
{
	float TotalWeight = 0.0f;

	for (const FRoomCandidate& Candidate : Candidates)
	{
		TotalWeight += Candidate.Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		return INDEX_NONE;
	}

	const float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);

	float Accumulated = 0.0f;
	for (int32 Index = 0; Index < Candidates.Num(); ++Index)
	{
		Accumulated += Candidates[Index].Weight;
		if (RandomValue <= Accumulated)
		{
			return Index;
		}
	}

	return Candidates.Num() - 1;
}
