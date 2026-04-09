// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/RoomData.h"
#include "Map/RoomActor/DoorDirection.h"
#include "RoomActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class ADoorActor;

UCLASS()
class ARoomActor : public AActor
{
	GENERATED_BODY()

public:
	ARoomActor();

	void InitializeRoom(const FRoomData& InRoomData);
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

	UPROPERTY()
	TArray<TObjectPtr<ADoorActor>> SpawnedDoors;

	UPROPERTY()
	FRoomData CachedRoomData;

private:
	void ClearSpawnedDoors();
	void SpawnConnectedDoors();
	void SpawnDoor(EDoorDirection Direction);

	FTransform GetDoorTransform(EDoorDirection Direction) const;
	FIntPoint GetNeighborGridPos(EDoorDirection Direction) const;
};
