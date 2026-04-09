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
	/* 연결된 문 위치를 기준으로 방 내부 보장 경로를 생성
	   - 문 1개: 문 -> 중심
	   - 문 2개: 문A -> 중심 -> 문B
	   - 문 3개 이상: 각 문 -> 중심 */
	void BuildGuaranteedPaths(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent) const;

	/* 방의 연결 정보를 기준으로 문 anchor 위치를 반환 반환 좌표는 방 기준 로컬 좌표 */
	TArray<FVector> GetDoorAnchors(const FRoomData& RoomData, float RoomHalfExtent) const;

	/* 방 중심 허브 위치를 반환 현재는 항상 로컬 원점 (0,0,0) */
	FVector GetHubPoint() const;
};
