// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/MapLayout.h"
#include "Map/Debug/MapManager.h"
#include "MapDebugActor.generated.h"

UCLASS()
class AMapDebugActor : public AActor
{
	GENERATED_BODY()
	
public:
	AMapDebugActor();

	/* MapManager가 맵 생성 후 다시 그리라고 호출하는 함수 */
	void RefreshDebugDraw();
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<AMapManager> MapManager;

	void DebugDrawMap(const FMapLayout& Layout);
};