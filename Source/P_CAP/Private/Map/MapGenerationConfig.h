// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapGenerationConfig.generated.h"

/**
 맵을 어떤 규칙으로 만들지 정해주는 설정 데이터 (맵 생성 규칙)
 */
USTRUCT(BlueprintType)
struct FMapGenerationConfig
{
	GENERATED_BODY()

public:
	/* 생성할 전체 방 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generation")
	int32 RoomCount = 10;

	/* 맵을 만들 때 사용할 시드 (입력값) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generation")
	int32 Seed = 12345;

	/* 시작방과 보스방 사이의 최소 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Generation")
	int32 MinBossDistance = 3;
	
	FMapGenerationConfig()
	{
	}
};
