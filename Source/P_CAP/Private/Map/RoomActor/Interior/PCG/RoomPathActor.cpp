// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Interior/PCG/RoomPathActor.h"

ARoomPathActor::ARoomPathActor()
{
	PrimaryActorTick.bCanEverTick = false;

	/* spline을 붙일 루트 생성 */
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	/* 실제 경로를 담을 spline 생성 */
	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	PathSpline->SetupAttachment(Root);
	PathSpline->SetClosedLoop(false);
	PathSpline->SetDrawDebug(true);
}

void ARoomPathActor::InitializePath(const TArray<FVector>& InWorldPathPoints)
{
	if (!PathSpline)
	{
		return;
	}

	/* 이전 경로가 있으면 지움 */
	PathSpline->ClearSplinePoints(false);

	/* 전달받은 월드 좌표들을 순서대로 spline point로 등록 */
	for (const FVector& WorldPoint : InWorldPathPoints)
	{
		PathSpline->AddSplinePoint(WorldPoint, ESplineCoordinateSpace::World, false);
	}

	/* 모든 점을 넣은 뒤 spline을 갱신 */
	PathSpline->UpdateSpline();
}
