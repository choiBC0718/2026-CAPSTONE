// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Map/MapLayout.h"
#include "Map/MapGenerator.h"
#include "Map/RoomActor/RoomActor.h"
#include "Map/RoomActor/DoorDirection.h"
#include "MapManager.generated.h"

class AStageExitActor;
class URoomMonsterSpawnDataAsset;
struct FStageConfig;

UCLASS()
class AMapManager : public AActor
{
	GENERATED_BODY()
	
public:
	AMapManager();

	/* 현재 생성된 맵 결과 반환 */
	const FMapLayout& GetCurrentLayout() const { return CurrentLayout; }

	/* 현재 사용 중인 시드 반환 */
	int32 GetCurrentSeed() const { return CurrentSeed; }

	/* 방 간격 반환 */
	float GetRoomSpacing() const { return RoomSpacing; }

	/* 현재 스폰된 RoomActor 중에서 그 좌표에 해당하는 방 반환 */
	ARoomActor* FindSpawnedRoomByGridPos(const FIntPoint& InGridPos) const;

	/* 플레이어를 실제 이동시키는 함수 */
	void RequestMovePlayer(ACharacter* PlayerCharacter, const FIntPoint& TargetRoomPos, EDoorDirection ExitDirection);

	void GenerateStage(const FStageConfig& StageConfig);
	void ClearCurrentStage();

	/* 가장 최근에 맵 생성에 적용된 K-Means 성향 — Blueprint 위젯에서 읽기용 */
	UPROPERTY(BlueprintReadOnly, Category="AI Tendency")
	FPlayerTendencyModifier LastAppliedTendency;

	/* true면 K-Means 무시하고 아래 수치를 직접 사용 (에디터 테스트용) */
	UPROPERTY(EditAnywhere, Category="AI Tendency|Manual Override")
	bool bUseManualTendency = false;

	UPROPERTY(EditAnywhere, Category="AI Tendency|Manual Override", meta=(EditCondition="bUseManualTendency"))
	FPlayerTendencyModifier ManualTendency;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UMapGenerator> MapGenerator;

	UPROPERTY(EditAnywhere, Category="Map")
	int32 CurrentSeed = 12345;

	UPROPERTY(EditAnywhere, Category="Map")
	int32 CurrentRoomCount = 10;

	UPROPERTY(EditAnywhere, Category="Map")
	bool bUseRandomSeedOnBeginPlay = false;

	UPROPERTY(EditAnywhere, Category="Map")
	bool bGenerateMapOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category="Map Spawn")
	TSubclassOf<ARoomActor> RoomActorClass;

	UPROPERTY(EditAnywhere, Category="Map Spawn")
	float RoomSpacing = 2200.f;

	UPROPERTY(EditAnywhere, Category="Stage Exit")
	TSubclassOf<AStageExitActor> StageExitActorClass;

	UPROPERTY(EditAnywhere, Category="Stage Exit")
	FVector StageExitLocalOffset = FVector(0.f, 0.f, 120.f);

	UPROPERTY()
	FMapLayout CurrentLayout;

	UPROPERTY()
	TArray<TObjectPtr<ARoomActor>> SpawnedRooms;

	UPROPERTY()
	TArray<TObjectPtr<AStageExitActor>> SpawnedStageExits;

	UPROPERTY()
	TObjectPtr<URoomMonsterSpawnDataAsset> CurrentMonsterSpawnDataAsset;

	/* 좌표 기반 빠른 조회용 */
	UPROPERTY()
	TMap<FIntPoint, TObjectPtr<ARoomActor>> SpawnedRoomMap;

	void BindInput();
	void RegenerateWithRandomSeed();
	void RegenerateWithCurrentSeed();

	void EnsureMapGenerator();
	void GenerateMapAndSpawnRooms();
	void SpawnRooms(const FMapLayout& Layout);
	void SpawnStageExitInRoom(ARoomActor* RoomActor, const FRoomData& RoomData);
	void MovePlayerToStartRoom();
	void ClearSpawnedStageExits();
	void ClearSpawnedRooms();
};
