// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Map/RoomData.h"
#include "RoomInteriorData.h"
#include "RoomInteriorGenerator.generated.h"

/**
 * 방 정보와 시드를 입력받아 방 내부 경로 데이터를 만들어주는 생성기 클래스
 * - 방의 연결 방향을 읽고
 * - 각 문의 로컬 위치(anchor)를 계산한 뒤
 * - 방 중심을 기준으로 보장 경로를 만든다
 */
UCLASS()
class URoomInteriorGenerator : public UObject
{
	GENERATED_BODY()

public:
	/* 방 내부 경로 생성을 시작하는 메인 함수
	   - 현재는 셀 배치 없이 GuaranteedPaths만 채워서 반환 */
	FRoomInteriorLayout GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const;

private:
	/* 시드 기반 랜덤 스트림 생성 */
	FRandomStream MakeRoomRandomStream(const FRoomData& RoomData, int32 MapSeed) const;

	/* 내부 셀 그리드의 기본 정보를 초기화 */
	void InitializeCellGrid(FRoomInteriorLayout& OutLayout, float RoomHalfExtent, float CellSize) const;

	/* 문 앞 안전 구역을 셀 단위로 예약 */
	void MarkDoorReservedCells(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent) const;

	/* 보장 이동 경로 주변 셀을 예약 */
	void MarkGuaranteedPathCells(FRoomInteriorLayout& OutLayout, float RoomHalfExtent) const;

	/* 방 연결 형태에 맞는 초기 큰 구조물 패턴을 배치 */
	void PlaceLargeStructurePattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream) const;

	/* 큰 골격 이후 남는 공간에 보조 구조물을 추가 배치 */
	void PlaceSecondaryStructures(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent, FRandomStream& RandomStream) const;

	/* 연결 형태별 패턴 적용 함수 */
	void PlaceDeadEndPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream) const;
	void PlaceStraightPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream) const;
	void PlaceCornerPattern(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, FRandomStream& RandomStream) const;
	void PlaceHubPattern(FRoomInteriorLayout& OutLayout, FRandomStream& RandomStream) const;

	/* 여러 후보 배치 중 가능한 조합을 순서대로 시도 */
	void PlaceCandidateStructures(
		FRoomInteriorLayout& OutLayout,
		const TArray<TArray<FRoomInteriorPlacedStructure>>& CandidateGroups,
		FRandomStream& RandomStream) const;

	/* 큰 구조물 footprint 배치 가능 여부 검사 */
	bool TryPlaceStructure(FRoomInteriorLayout& OutLayout, const FIntPoint& Origin, const FIntPoint& Footprint) const;

	/* footprint 주변 최소 간격 영역이 비어 있는지 검사 */
	bool HasRequiredStructureSpacing(
		const FRoomInteriorLayout& Layout,
		const FIntPoint& Origin,
		const FIntPoint& Footprint,
		int32 RequiredGapInCells) const;

	/* 배치 후 연결성까지 포함해 구조물 확정 시도 */
	bool TryPlaceStructureWithValidation(
		FRoomInteriorLayout& OutLayout,
		const FRoomData& RoomData,
		float RoomHalfExtent,
		const FIntPoint& Origin,
		const FIntPoint& Footprint,
		ERoomInteriorStructureCategory Category) const;

	/* 현재 셀 레이아웃에서 모든 문이 서로 도달 가능한지 검사 */
	bool AreAllDoorsReachable(const FRoomInteriorLayout& Layout, const FRoomData& RoomData, float RoomHalfExtent) const;

	/* 로컬 좌표를 셀 좌표로 변환 */
	FIntPoint LocalPointToCell(const FRoomInteriorLayout& Layout, const FVector& LocalPoint, float RoomHalfExtent) const;

	/* 셀 상태 읽기/쓰기 보조 함수 */
	bool IsValidCell(const FRoomInteriorLayout& Layout, const FIntPoint& Coord) const;
	int32 GetCellIndex(const FRoomInteriorLayout& Layout, const FIntPoint& Coord) const;
	ERoomInteriorCellType GetCellType(const FRoomInteriorLayout& Layout, const FIntPoint& Coord) const;
	void SetCellType(FRoomInteriorLayout& OutLayout, const FIntPoint& Coord, ERoomInteriorCellType NewType) const;

	/* 연결된 문 위치를 기준으로 방 내부 보장 경로를 생성
	   - 문 1개: 문 -> 중심 + 중심 기준 원형 경로
	   - 문 2개: 문A -> 중심 -> 문B
	   - 문 3개 이상: 각 문 -> 중심 */
	void BuildGuaranteedPaths(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent) const;

	/* 막다른 방 전용 원형 경로를 생성 */
	FRoomInteriorPath BuildDeadEndLoopPath(float RoomHalfExtent, float CorridorWidth) const;

	/* 방의 연결 정보를 기준으로 문 anchor 위치를 반환
	   - 반환 좌표는 방 기준 로컬 좌표 */
	TArray<FVector> GetDoorAnchors(const FRoomData& RoomData, float RoomHalfExtent) const;

	/* 방 중심 허브 위치를 반환
	   - 현재는 항상 로컬 원점 (0,0,0) */
	FVector GetHubPoint() const;
};
