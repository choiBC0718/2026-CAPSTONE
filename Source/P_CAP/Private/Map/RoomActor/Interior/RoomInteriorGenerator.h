// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Map/RoomData.h"
#include "RoomInteriorData.h"
#include "RoomInteriorGenerator.generated.h"

/**
 * 방 정보와 시드를 입력받아 문 앞을 피한 셀 기반 내부 배치 결과를 만들어주는 생성기 클래스
 * - 방 크기를 보고 셀 그리드 만들기
 * - 같은 시드 기준으로 장애물 셀 선택하기
 * - 최종 FRoomInteriorLayout 반환하기
 */
UCLASS()
class URoomInteriorGenerator : public UObject
{
	GENERATED_BODY()

public:
	/* 방 내부 배치 생성 전체를 시작하는 메인 함수 */
	FRoomInteriorLayout GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const;

private:
	/* 방 좌표와 타입을 반영해서 방마다 고유한 랜덤 시드를 만드는 함수 */
	int32 MakeRoomSeed(const FRoomData& RoomData, int32 MapSeed) const;

	/* 방 안을 셀로 나누고 기본 셀 데이터를 만드는 함수 */
	void BuildCells(FRoomInteriorLayout& OutLayout, float RoomHalfExtent, float CellSize, float Margin) const;

	/* 문 앞 셀을 사용 금지(Reserved) 상태로 바꾸는 함수 */
	void ReserveDoorCells(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData) const;

	/* 빈 셀 중 일부를 장애물로 바꾸는 함수 */
	void PlaceObstacles(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, int32 RoomSeed) const;

	/* 지정 범위 셀을 한꺼번에 예약하는 보조 함수 */
	void ReserveCellsRect(FRoomInteriorLayout& OutLayout, int32 MinX, int32 MaxX, int32 MinY, int32 MaxY) const;
};
