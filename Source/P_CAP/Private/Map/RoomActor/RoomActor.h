// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
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

	/* 문을 방 가장자리 어디에 놓을지 계산할 때 쓰는 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Door")
	float RoomHalfExtent = 1000.f;

	/* 문 높이 조절하는 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Door")
	float DoorSpawnZOffset = 0.f;

	/* 문을 안쪽으로 배치할 때 조절하는 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Door")
	float DoorInset = 0.f;

	/* 내부 셀 한 칸 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	float InteriorCellSize = 250.f;

	/* 벽 쪽 여유 공간 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	float InteriorMargin = 200.f;

	/* 장애물 메시 높이 보정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	float ObstacleHeightOffset = 50.f;

	/* 장애물 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	UStaticMesh* ObstacleMesh = nullptr;
	
	UPROPERTY()
	TArray<TObjectPtr<ADoorActor>> SpawnedDoors;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> SpawnedInteriorMeshes;
	
	UPROPERTY()
	FRoomData CachedRoomData;

	/* 내부 생성 시 사용할 맵 시드 보관 */
	UPROPERTY()
	int32 CachedMapSeed = 0;

	/* 내부 배치 생성기 */
	UPROPERTY()
	TObjectPtr<URoomInteriorGenerator> InteriorGenerator;
	
private:
	void ClearSpawnedDoors();
	void SpawnConnectedDoors();
	void SpawnDoor(EDoorDirection Direction);

	void ClearSpawnedInteriorMeshes();
	void GenerateAndSpawnInterior();
	void SpawnObstacleMeshAtLocalPosition(const FVector& LocalPosition);
	
	FTransform GetDoorTransform(EDoorDirection Direction) const;
	FIntPoint GetNeighborGridPos(EDoorDirection Direction) const;
};
