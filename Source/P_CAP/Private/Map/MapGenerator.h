// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MapGenerationConfig.h"
#include "MapLayout.h"
#include "MapGenerator.generated.h"

/**
 맵 생성 설정을 바탕으로 한 스테이지의 맵 구조를 생성하는 클래스
 */
UCLASS(BlueprintType)
class UMapGenerator : public UObject
{
	GENERATED_BODY()

public:
	/* 맵 생성 설정을 바탕으로 시작방 생성, 방 확장,
	   연결 설정, 거리 계산, 보스방 지정까지 수행해 최종 맵 데이터를 반환 */
	UFUNCTION(BlueprintCallable, Category = "Map Generation")
	FMapLayout GenerateMap(const FMapGenerationConfig& InConfig);

private:
	/* (0,0) 위치에 시작방을 생성하고 맵 데이터에 등록 */
	void CreateStartRoom(FMapLayout& OutLayout);

	/* 기존 방들 중 하나를 랜덤으로 선택하고
	   비어 있는 인접 칸에 새 방을 추가해 목표 방 개수까지 맵을 확장 */
	void ExpandRooms(FMapLayout& OutLayout, FRandomStream& RandomStream);

	/* 모든 방을 돌면서 상하좌우에 인접한 방이 있는지 검사해 문 연결 정보를 설정 */
	void UpdateRoomConnections(FMapLayout& OutLayout);

	/* 시작방에서부터 BFS로 맵 전체를 탐색하며 각 방까지의 최단 거리를 계산
	   (이 값을 통해 특수 방이나 보스 방을 지정할 수 있음) */
	void CalculateDistanceFromStart(FMapLayout& OutLayout);

	/* 시작방을 제외한 방들 중 가장 먼 방을 찾아 보스방으로 지정 */
	void AssignBossRoom(FMapLayout& OutLayout);

	/* 시작방과 보스방을 제외한 방들 중 나머지 특수방 지정 */
	void AssignSpecialRooms(FMapLayout& OutLayout, FRandomStream& RandomStream);

	/* 현재 좌표를 기준으로 상하좌우 인접 좌표 4개를 반환 */
	TArray<FIntPoint> GetNeighborPositions(const FIntPoint& InPos) const;

	/* 현재 좌표 주변 상하좌우 중 방이 없는 빈 좌표만 반환 */
	TArray<FIntPoint> GetAvailableNeighborPositions(const FMapLayout& InLayout, const FIntPoint& InPos) const;

	/* 특수방 후보가 될 수 있는지 검사 하는 함수 */
	bool IsSpecialRoomCandidate(const FMapLayout& InLayout, const FRoomData& Room) const;

	/* 후보 배열 안에서 랜덤으로 하나를 뽑는 함수 */
	int32 PickRandomCandidateIndex(const TArray<FIntPoint>& Candidates, FRandomStream& RandomStream) const;

	/* 시작방, 보스방 제외 사용 가능한 막다른 방의 개수를 세는 함수 */
	int32 CountAvailableDeadEnds(const FMapLayout& InLayout) const;
private:
	struct FRoomCandidate
	{
		FIntPoint BaseRoomPos;
		FIntPoint NewRoomPos;
		float Weight = 1.0f;
	};

	/* 후보 위치 주변 상하좌우에 몇 개의 방이 붙어 있는지 계산 */
	int32 CountAdjacentRooms(const FMapLayout& InLayout, const FIntPoint& InPos) const;

	/* InPos에 방을 추가했을 때 2x2 블록이 생기는지 검사 */
	bool WouldCreate2x2Block(const FMapLayout& InLayout, const FIntPoint& InPos) const;

	/* 후보 위치의 가중치 계산 */
	float CalculateCandidateWeight(const FMapLayout& InLayout, const FIntPoint& BaseRoomPos, const FIntPoint& CandidatePos) const;

	/* 가중치 랜덤으로 후보 하나 선택 */
	int32 PickWeightedCandidateIndex(const TArray<FRoomCandidate>& Candidates, FRandomStream& RandomStream) const;
};
