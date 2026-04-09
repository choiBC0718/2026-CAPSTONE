// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomInteriorData.generated.h"

/**
 * 방 내부의 생성 결과를 저장하는 데이터 구조
 */

/* 방 내부 보장 이동 경로 1개의 정보 */
USTRUCT(BlueprintType)
struct FRoomInteriorPath
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> PathPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorridorWidth = 0.f;
};

/* 방 하나의 내부 배치 전체 */
USTRUCT(BlueprintType)
struct FRoomInteriorLayout
{
	GENERATED_BODY()

	/* 문 위치를 기준으로 만든 보장 경로 배열 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoomInteriorPath> GuaranteedPaths;
};
