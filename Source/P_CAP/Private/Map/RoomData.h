// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomTypes.h"
#include "RoomData.generated.h"

/**
 맵에서 하나의 방이 위치, 연결, 타입 등의 정보를 가지도록 표현하는 데이터 (방 1개의 정보)
 */
USTRUCT(BlueprintType)
struct FRoomData
{
	GENERATED_BODY()

public:
	/* 방의 격자 좌표 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	FIntPoint GridPos = FIntPoint::ZeroValue;

	/* 방 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	ERoomType RoomType = ERoomType::Normal;

	/* 문 연결 정보 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Connection")
	bool bConnectedUp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Connection")
	bool bConnectedDown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Connection")
	bool bConnectedLeft = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room|Connection")
	bool bConnectedRight = false;

	/* 시작방으로부터 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	int32 DistanceFromStart = -1;

	/* 객체를 편하게 만들기 위해 생성자 선언 */
	FRoomData()
	{
	}

	FRoomData(const FIntPoint& InGridPos)
		: GridPos(InGridPos)
	{
	}

	/* 현재 방이 상하좌우로 몇 개의 방과 연결되어 있는지 반환 */
	int32 GetConnectionCount() const
	{
		int32 Count = 0;
		Count += bConnectedUp ? 1 : 0;
		Count += bConnectedDown ? 1 : 0;
		Count += bConnectedLeft ? 1 : 0;
		Count += bConnectedRight ? 1 : 0;
		return Count;
	}

	/* 연결된 방향이 하나뿐인지 확인하여 막다른 방인지 판단 */
	bool IsDeadEnd() const
	{
		return GetConnectionCount() == 1;
	}
};