// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "RoomPathActor.generated.h"

UCLASS()
class ARoomPathActor : public AActor
{
	GENERATED_BODY()

public:
	ARoomPathActor();

	/* 월드 좌표 경로 점을 받아 spline을 구성하는 초기화 함수 */
	void InitializePath(const TArray<FVector>& InWorldPathPoints);

protected:
	/* PathSpline의 부모가 되는 루트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	USceneComponent* Root;

	/* 실제 경로를 보관하는 spline 컴포넌트 현재는 SetDrawDebug(true)로 확인용 선을 표시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	USplineComponent* PathSpline;
};
