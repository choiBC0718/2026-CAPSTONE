// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"

FRoomInteriorLayout URoomInteriorGenerator::GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const
{
	/* 최종 결과를 담을 내부 레이아웃 */
	FRoomInteriorLayout Layout;

	/* 셀 배치 대신 연결 경로만 계산 */
	BuildGuaranteedPaths(Layout, RoomData, RoomHalfExtent);

	return Layout;
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

	/* 입구만 있는 dead-end 방은 문에서 중심으로 들어오는 경로 1개만 생성 */
	if (DoorAnchors.Num() == 1)
	{
		FRoomInteriorPath Path;
		Path.CorridorWidth = CorridorWidth;
		Path.PathPoints.Add(DoorAnchors[0]);
		Path.PathPoints.Add(HubPoint);
		OutLayout.GuaranteedPaths.Add(Path);
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
