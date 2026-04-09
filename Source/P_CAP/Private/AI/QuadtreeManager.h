// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuadtreeManager.generated.h"

struct FQuadtreeNode
{
	FVector Center;       // 구역의 중심 좌표
	FVector Extent;       // 구역의 절반 크기 
	int32 NodeID;         // 구역 고유 번호
	int32 VisitCount;     // 방문 횟수 장부

	FQuadtreeNode* Children[4];

	FQuadtreeNode(FVector InCenter, FVector InExtent)
	{
		Center = InCenter;
		Extent = InExtent;
		NodeID = -1;
		VisitCount = 0;
		for (int32 i = 0; i < 4; ++i) Children[i] = nullptr;
	}

	~FQuadtreeNode()
	{
		for (int32 i = 0; i < 4; ++i)
		{
			if (Children[i]) delete Children[i];
		}
	}
};

UCLASS()
class P_CAP_API AQuadtreeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AQuadtreeManager();

	// 전체 맵의 크기 (X, Y 넓이)와 Z축 두께(높이)
	UPROPERTY(EditAnywhere, Category = "Quadtree")
	FVector MapSize = FVector(2000.f, 2000.f, 100.f); 

	UPROPERTY(EditAnywhere, Category = "Quadtree")
	int32 MaxDepth = 4; // 쪼개는 횟수

	// 플레이어 트래커가 호출할 방문 기록 함수
	void AddVisit(FVector PlayerLocation);

	// AI 학습기가 데이터를 긁어갈 때 호출할 함수
	TArray<FQuadtreeNode*> GetAllLeafNodes();

protected:
	virtual void BeginPlay() override;

private:
	FQuadtreeNode* RootNode;
	int32 CurrentNodeID = 0;

	// 공간을 4등분하는 재귀 함수
	void Subdivide(FQuadtreeNode* Node, int32 Depth);
	
	void UpdateNodeVisit(FQuadtreeNode* Node, FVector PlayerLocation);
	void CollectLeafNodes(FQuadtreeNode* Node, TArray<FQuadtreeNode*>& OutNodes);
	void DrawQuadtree(FQuadtreeNode* Node);
};