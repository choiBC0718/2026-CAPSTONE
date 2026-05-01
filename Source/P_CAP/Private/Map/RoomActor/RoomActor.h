// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomActor/Interior/RoomInteriorPropSet.h"
#include "Map/RoomActor/Interior/PCG/RoomPathActor.h"
#include "Map/RoomData.h"
#include "Map/RoomActor/DoorDirection.h"
#include "Map/RoomActor/DoorActor.h"
#include "RoomActor.generated.h"

UCLASS()
class ARoomActor : public AActor
{
	GENERATED_BODY()

public:
	ARoomActor();

	void InitializeRoom(const FRoomData& InRoomData, int32 InMapSeed);
	FVector GetEntrancePoint(EDoorDirection Direction) const;
	virtual void Destroyed() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
	UStaticMeshComponent* FloorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Door")
	TSubclassOf<ADoorActor> DoorActorClass;

	/* 문을 방 가장자리 어디에 붙일지 계산할 때 쓰는 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Door")
	float RoomHalfExtent = 1000.f;

	/* 문 높이 조절값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Door")
	float DoorSpawnZOffset = 0.f;

	/* 문을 안쪽으로 배치할 때 사용하는 조절값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Door")
	float DoorInset = 0.f;

	/* 내부 생성 함수 시그니처를 맞추기 위해 유지하는 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	float InteriorCellSize = 250.f;

	/* 내부 생성 함수 시그니처를 맞추기 위해 유지하는 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	float InteriorMargin = 200.f;

	/* 방 바닥에서 경로를 띄워 그릴 높이 spline이 바닥과 겹쳐 안 보이는 것을 방지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	float PathZOffset = 10.f;

	/* 내부 셀 디버그 박스 표시 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	bool bDrawInteriorCellDebug = true;

	/* 큰 구조물 메쉬 후보를 담는 데이터 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	TObjectPtr<URoomInteriorPropSet> LargeStructurePropSet;
	
	UPROPERTY()
	TArray<TObjectPtr<ADoorActor>> SpawnedDoors;

	/* 현재 방에서 생성한 경로 액터 목록
	   - 방이 다시 초기화될 때 함께 정리 */
	UPROPERTY()
	TArray<TObjectPtr<ARoomPathActor>> SpawnedPathActors;

	/* 이 방에서 동적으로 생성한 큰 구조물 메쉬 컴포넌트 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> SpawnedStructureMeshes;

	UPROPERTY()
	FRoomData CachedRoomData;

	/* 내부 생성에 사용한 맵 시드 보관 */
	UPROPERTY()
	int32 CachedMapSeed = 0;

	/* 마지막 내부 생성 결과를 캐싱 */
	UPROPERTY()
	FRoomInteriorLayout CachedInteriorLayout;

	/* 내부 경로 생성기 */
	UPROPERTY()
	TObjectPtr<URoomInteriorGenerator> InteriorGenerator;

private:
	void ClearSpawnedDoors();
	/* 이 방이 소유하는 경로 액터들을 정리 */
	void ClearSpawnedPathActors();
	/* 이 방이 소유하는 큰 구조물 메쉬 컴포넌트를 정리 */
	void ClearSpawnedStructureMeshes();
	void SpawnConnectedDoors();
	void SpawnDoor(EDoorDirection Direction);

	/* 내부 경로 데이터를 생성하고 실제 path actor를 배치 */
	void GenerateAndSpawnInterior();
	/* 생성된 경로 데이터를 바탕으로 전용 path actor를 스폰 */
	void SpawnGuaranteedPaths(const FRoomInteriorLayout& Layout);
	/* 큰 구조물 점유 결과를 실제 메쉬 컴포넌트로 배치 */
	void SpawnLargeStructureMeshes(const FRoomInteriorLayout& Layout);
	/* 셀 기반 구조 결과를 디버그 박스로 시각화 */
	void DrawInteriorCellDebug(const FRoomInteriorLayout& Layout) const;
	
	FTransform GetDoorTransform(EDoorDirection Direction) const;
	FIntPoint GetNeighborGridPos(EDoorDirection Direction) const;
};
