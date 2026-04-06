// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomInteriorData.generated.h"

/**
 * 방 내부 랜덤 배치 결과를 저장하는 데이터 구조
 */

/* 셀의 상태를 나타내는 enum */
UENUM(BlueprintType)
enum class ERoomInteriorCellType : uint8
{
	Empty		UMETA(DisplayName = "Empty"),
	Reserved	UMETA(DisplayName = "Reserved"),
	Obstacle	UMETA(DisplayName = "Obstacle")
};

/* 셀 하나의 정보 */
USTRUCT(BlueprintType)
struct FRoomInteriorCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint CellCoord = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocalPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERoomInteriorCellType CellType = ERoomInteriorCellType::Empty;
};

/* 방 하나의 내부 배치 전체 */
USTRUCT(BlueprintType)
struct FRoomInteriorLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoomInteriorCell> Cells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CellsPerSide = 0;

	/* 2차원 좌표를 1차원 배열 인덱스로 바꾸는 함수 */
	int32 GetCellIndex(int32 X, int32 Y) const
	{
		return Y * CellsPerSide + X;
	}

	/* 좌표가 범위 안인지 확인하는 bool 함수 */
	bool IsValidCell(int32 X, int32 Y) const
	{
		return X >= 0 && X < CellsPerSide && Y >= 0 && Y < CellsPerSide;
	}

	FRoomInteriorCell* FindCell(int32 X, int32 Y)
	{
		if (!IsValidCell(X, Y))
		{
			return nullptr;
		}

		const int32 Index = GetCellIndex(X, Y);
		return Cells.IsValidIndex(Index) ? &Cells[Index] : nullptr;
	}

	/* (X,Y) 셀을 바로 찾아주는 함수 */
	const FRoomInteriorCell* FindCell(int32 X, int32 Y) const
	{
		if (!IsValidCell(X, Y))
		{
			return nullptr;
		}

		const int32 Index = GetCellIndex(X, Y);
		return Cells.IsValidIndex(Index) ? &Cells[Index] : nullptr;
	}
};