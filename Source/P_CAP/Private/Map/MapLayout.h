// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomData.h"
#include "MapGenerationConfig.h"
#include "MapLayout.generated.h"

/**
 한 스테이지에 생성된 전체 맵 결과를 저장하는 데이터 구조
 */
USTRUCT(BlueprintType)
struct FMapLayout
{
	GENERATED_BODY()

public:
	/* 생성된 모든 방 데이터 (Key: 격자 좌표, Value: 방 정보) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Layout")
	TMap<FIntPoint, FRoomData> Rooms;

	/* 시작방 좌표 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Layout")
	FIntPoint StartRoomPos = FIntPoint::ZeroValue;

	/* 보스방 좌표 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Layout")
	FIntPoint BossRoomPos = FIntPoint::ZeroValue;

	/* 실제 생성에 사용된 시드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Layout")
	int32 UsedSeed = 0;

	/* 실제 맵 생성에 사용된 시드 (결과 기록) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Layout")
	FMapGenerationConfig UsedConfig;

	FMapLayout()
	{
	}

	/* 현재 맵에 생성된 총 방 개수 반환 */
	int32 GetRoomCount() const
	{
		return Rooms.Num();
	}

	/* 특정 좌표에 방이 존재하는지 확인 (중복 생성 방지) */
	bool HasRoom(const FIntPoint& InPos) const
	{
		return Rooms.Contains(InPos);
	}

	/* 특정 좌표의 방 데이터를 찾음 (없으면 nullptr 반환) */
	FRoomData* FindRoom(const FIntPoint& InPos)
	{
		return Rooms.Find(InPos);
	}

	/* 특정 좌표의 방 데이터를 찾음 const 버전 (읽기 전용, 안정성을 위해) */
	const FRoomData* FindRoom(const FIntPoint& InPos) const
	{
		return Rooms.Find(InPos);
	}
};