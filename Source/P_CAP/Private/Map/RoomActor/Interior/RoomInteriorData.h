// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomInteriorData.generated.h"

/**
 * 방 내부의 생성 결과를 저장하는 데이터 구조
 */

UENUM(BlueprintType)
enum class ERoomInteriorCellType : uint8
{
	Empty UMETA(DisplayName = "Empty"),
	ReservedDoor UMETA(DisplayName = "Reserved Door"),
	ReservedPath UMETA(DisplayName = "Reserved Path"),
	Blocked UMETA(DisplayName = "Blocked")
};

UENUM(BlueprintType)
enum class ERoomInteriorStructureCategory : uint8
{
	Generic UMETA(DisplayName = "Generic"),
	Corner UMETA(DisplayName = "Corner"),
	Side UMETA(DisplayName = "Side"),
	EndCap UMETA(DisplayName = "End Cap"),
	Hub UMETA(DisplayName = "Hub")
};

USTRUCT(BlueprintType)
struct FRoomInteriorCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Coord = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERoomInteriorCellType Type = ERoomInteriorCellType::Empty;
};

USTRUCT(BlueprintType)
struct FRoomInteriorPlacedStructure
{
	GENERATED_BODY()

	FRoomInteriorPlacedStructure() = default;

	FRoomInteriorPlacedStructure(
		const FIntPoint& InOrigin,
		const FIntPoint& InFootprint,
		ERoomInteriorStructureCategory InCategory = ERoomInteriorStructureCategory::Generic)
		: Origin(InOrigin)
		, Footprint(InFootprint)
		, Category(InCategory)
	{
	}

	/* 점유 시작 셀 좌표 (좌하단 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Origin = FIntPoint::ZeroValue;

	/* 점유 footprint 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Footprint = FIntPoint(1, 1);

	/* 구조물 카테고리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERoomInteriorStructureCategory Category = ERoomInteriorStructureCategory::Generic;
};

/* 방 내부 보장 이동 경로 1개의 정보
   - PathPoints: 문/중심을 잇는 경로 점 목록
   - CorridorWidth: 나중에 PCG에서 경로 주변 금지 폭으로 활용 가능 */
USTRUCT(BlueprintType)
struct FRoomInteriorPath
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> PathPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorridorWidth = 0.f;

	/* true면 마지막 점과 첫 점을 이어 닫힌 경로로 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bClosedLoop = false;
};

/* 방 하나의 내부 배치 전체
   - 현재는 셀 정보 없이 경로 정보만 보관 */
USTRUCT(BlueprintType)
struct FRoomInteriorLayout
{
	GENERATED_BODY()

	/* 내부 그리드 가로/세로 셀 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridWidth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridHeight = 0;

	/* 셀 한 칸의 월드 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CellSize = 0.f;

	/* 셀 단위 내부 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoomInteriorCell> Cells;

	/* 문 위치를 기준으로 만든 보장 경로 배열 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoomInteriorPath> GuaranteedPaths;

	/* 큰 구조물용 점유 영역 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoomInteriorPlacedStructure> PlacedStructures;
};
