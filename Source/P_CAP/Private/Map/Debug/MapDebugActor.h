// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/MapGenerator.h"
#include "Map/MapLayout.h"
#include "Map/RoomActor/RoomActor.h"
#include "Map/RoomActor/DoorDirection.h"
#include "MapDebugActor.generated.h"

UCLASS()
class AMapDebugActor : public AActor
{
	GENERATED_BODY()
	
public:
	AMapDebugActor();

	/* 지금 월드에 스폰된 RoomActor 중에서 그 좌표에 해당하는 방 */
	ARoomActor* FindSpawnedRoomByGridPos(const FIntPoint& InGridPos) const;
	/* 플레이어를 실제 이동 시키는 함수 */
	void RequestMovePlayer(class ACharacter* PlayerCharacter, const FIntPoint& TargetRoomPos, EDoorDirection ExitDirection);
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UMapGenerator* MapGenerator;

	/* 에디터에서 직접 입력할 시드 */
	UPROPERTY(EditAnywhere, Category="Map Debug")
	int32 CurrentSeed = 12345;

	/* 방 개수 */
	UPROPERTY(EditAnywhere, Category="Map Debug")
	int32 CurrentRoomCount = 10;

	/* 시작 시 랜덤 시드 사용할지 체크 박스 */
	UPROPERTY(EditAnywhere, Category="Map Debug")
	bool bUseRandomSeedOnBeginPlay = false;
	
	UPROPERTY(EditAnywhere, Category="Map Spawn")
	TSubclassOf<ARoomActor> RoomActorClass;

	UPROPERTY(EditAnywhere, Category="Map Spawn")
	float RoomSpacing = 2200.f;

	UPROPERTY()
	TArray<TObjectPtr<ARoomActor>> SpawnedRooms;
	
	void GenerateAndDebug();
	void DebugDrawMap(const FMapLayout& Layout);

	void BindDebugInput();
	void RegenerateWithRandomSeed();
	void RegenerateWithCurrentSeed();

	void SpawnRooms(const FMapLayout& Layout);
	void ClearSpawnedRooms();
};