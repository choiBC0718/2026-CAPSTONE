// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerTrackerComponent.generated.h"

class AQuadtreeManager;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPlayerTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	void RecordLocation();
	FTimerHandle TrackingTimer;

	int32 PassedObstacleCount = 0;
	int32 AvoidedObstacleCount = 0;

protected:
	virtual void BeginPlay() override;

private:
	// 맵에서 찾은 옥트리 매니저를 기억할 포인터
	UPROPERTY()
	AQuadtreeManager* CachedQuadtreeManager;
	UPROPERTY(EditAnywhere)
	bool bDrawDebug = false;
};