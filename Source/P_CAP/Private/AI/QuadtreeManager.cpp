// Fill out your copyright notice in the Description page of Project Settings.

#include "QuadtreeManager.h"
#include "DrawDebugHelpers.h"

AQuadtreeManager::AQuadtreeManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AQuadtreeManager::BeginPlay()
{
	Super::BeginPlay();

	// 루트 노드 생성 -> 액터의 현재 위치를 중심으로 맵 생성
	RootNode = new FQuadtreeNode(GetActorLocation(), MapSize);
	CurrentNodeID = 0;

	// 쿼드트리 쪼개기
	Subdivide(RootNode, 0);

	// 시각적 확인을 위해 디버그 박스 그리기
	DrawQuadtree(RootNode);
	UE_LOG(LogTemp, Warning, TEXT("쿼드트리 생성 완료! 총 %d개의 평면 구역이 만들어졌습니다."), CurrentNodeID);
}

void AQuadtreeManager::Subdivide(FQuadtreeNode* Node, int32 Depth)
{
	if (Depth >= MaxDepth)
	{
		Node->NodeID = CurrentNodeID++;
		return;
	}
	FVector NewExtent = Node->Extent;
	NewExtent.X /= 2.0f;
	NewExtent.Y /= 2.0f;

	// 사분면의 중심점 좌표를 계산
	FVector Offsets[4] = {
		FVector(NewExtent.X, NewExtent.Y, 0.f),   // 1사분면
		FVector(NewExtent.X, -NewExtent.Y, 0.f),  // 4사분면
		FVector(-NewExtent.X, NewExtent.Y, 0.f),  // 2사분면
		FVector(-NewExtent.X, -NewExtent.Y, 0.f)  // 3사분면
	};

	// 4개의 자식 노드를 생성하고 재귀적으로 쪼갬
	for (int32 i = 0; i < 4; ++i)
	{
		FVector ChildCenter = Node->Center + Offsets[i];
		Node->Children[i] = new FQuadtreeNode(ChildCenter, NewExtent);
		Subdivide(Node->Children[i], Depth + 1);
	}
}

void AQuadtreeManager::AddVisit(FVector PlayerLocation)
{
	if (RootNode)
	{
		UpdateNodeVisit(RootNode, PlayerLocation);
	}
}

void AQuadtreeManager::UpdateNodeVisit(FQuadtreeNode* Node, FVector PlayerLocation)
{
	if (Node == nullptr) return;

	// 플레이어 위치가 이 노드 영역 안에 있는지 검사
	FBox NodeBox(Node->Center - Node->Extent, Node->Center + Node->Extent);
	if (!NodeBox.IsInside(PlayerLocation)) return;

	// 리프 노드라면 방문 횟수 증가
	if (Node->Children[0] == nullptr)
	{
		Node->VisitCount++;
		UE_LOG(LogTemp, Log, TEXT("플레이어가 [%d]번 구역에 진입했습니다! (현재 방문 횟수: %d)"), Node->NodeID, Node->VisitCount);
		// 밟은 타일은 초록색으로 표시
		DrawDebugBox(GetWorld(), Node->Center, Node->Extent, FColor::Green, true, -1.0f, 0, 5.0f);
		return;
	}

	// 리프 노드가 아니라면 4개의 자식 중 어디에 있는지 계속 파고듦
	for (int32 i = 0; i < 4; ++i)
	{
		UpdateNodeVisit(Node->Children[i], PlayerLocation);
	}
}

TArray<FQuadtreeNode*> AQuadtreeManager::GetAllLeafNodes()
{
	TArray<FQuadtreeNode*> LeafNodes;
	CollectLeafNodes(RootNode, LeafNodes);
	return LeafNodes;
}

void AQuadtreeManager::CollectLeafNodes(FQuadtreeNode* Node, TArray<FQuadtreeNode*>& OutNodes)
{
	if (Node == nullptr) return;

	if (Node->Children[0] == nullptr)
	{
		OutNodes.Add(Node);
		return;
	}

	for (int32 i = 0; i < 4; ++i)
	{
		CollectLeafNodes(Node->Children[i], OutNodes);
	}
}

void AQuadtreeManager::DrawQuadtree(FQuadtreeNode* Node)
{
	if (Node == nullptr) return;

	if (Node->Children[0] == nullptr)
	{
		// 평면 타일들을 빨간색 박스로 그려줌
		DrawDebugBox(GetWorld(), Node->Center, Node->Extent, FColor::Red, true, -1.0f, 0, 2.0f);
		return;
	}

	for (int32 i = 0; i < 4; ++i)
	{
		DrawQuadtree(Node->Children[i]);
	}
}