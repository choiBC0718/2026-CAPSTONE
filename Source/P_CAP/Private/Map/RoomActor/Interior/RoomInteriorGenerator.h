// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Map/RoomData.h"
#include "RoomInteriorData.h"
#include "RoomInteriorGenerator.generated.h"

/**
 * 방 정보와 시드를 입력받아 방 내부 경로 데이터를 만들어주는 생성기 클래스
 */
UCLASS()
class URoomInteriorGenerator : public UObject
{
	GENERATED_BODY()

public:
	/* 방 내부 경로 생성을 시작하는 메인 함수 */
	FRoomInteriorLayout GenerateInteriorLayout(const FRoomData& RoomData, float RoomHalfExtent, float CellSize, float Margin, int32 MapSeed) const;

private:
	/* 연결된 문 위치를 기준으로 방 내부 보장 경로를 생성 */
	void BuildGuaranteedPaths(FRoomInteriorLayout& OutLayout, const FRoomData& RoomData, float RoomHalfExtent) const;

	/* 방의 연결 정보를 기준으로 문 anchor 위치를 반환 */
	TArray<FVector> GetDoorAnchors(const FRoomData& RoomData, float RoomHalfExtent) const;

	/* 방 중심 허브 위치를 반환 */
	FVector GetHubPoint() const;
};
