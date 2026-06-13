// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Map/RoomActor/Interior/RoomInteriorGenerator.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "Map/RoomActor/Interior/RoomInteriorPropSet.h"
#include "Map/RoomActor/Interior/PCG/RoomPathActor.h"
#include "Map/RoomActor/Monster/RoomMonsterSpawnerComponent.h"
#include "Map/RoomData.h"
#include "Map/RoomActor/DoorDirection.h"
#include "Map/RoomActor/DoorActor.h"
#include "AI/AnalysisObstacle.h"
#include "AI/PlayerBehaviorLearner.h"
#include "RoomActor.generated.h"

class URoomMonsterSpawnDataAsset;
class URoomSizeSettings;

UCLASS()
class ARoomActor : public AActor
{
	GENERATED_BODY()

public:
	ARoomActor();

	void InitializeRoom(
		const FRoomData& InRoomData,
		int32 InMapSeed,
		URoomMonsterSpawnDataAsset* InMonsterSpawnDataAsset = nullptr,
		const FPlayerTendencyModifier& InTendency = FPlayerTendencyModifier{},
		ERoomZone InZone = ERoomZone::Mid,
		URoomSizeSettings* InRoomSizeSettings = nullptr);
	void SetCombatRewardType(ECombatRoomRewardType NewRewardType);
	bool IsRoomCleared() const { return bRoomCleared; }
	FIntPoint GetGridPos() const { return CachedRoomData.GridPos; }
	ECombatRoomRewardType GetCombatRewardType() const { return CachedRoomData.CombatRewardType; }
	void ApplyPersistentClearedState();
	FVector GetEntrancePoint(EDoorDirection Direction) const;
	FTransform GetStageExitSpawnTransform() const;
	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable, Category="Room")
	void ActivateRoom(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category="Room")
	void DeactivateRoom();

	UFUNCTION(BlueprintPure, Category="Room")
	bool IsRoomActivated() const { return bRoomActivated; }

	UPROPERTY(EditDefaultsOnly, Category="Room|Reward")
	TSubclassOf<class ACAP_RewardChest> RewardChestClass;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void SpawnRewardChest();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room")
	UStaticMeshComponent* FloorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room|Trigger")
	TObjectPtr<UBoxComponent> RoomEnterTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room|Monster")
	TObjectPtr<URoomMonsterSpawnerComponent> MonsterSpawnerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room|Stage Exit")
	TObjectPtr<USceneComponent> StageExitSpawnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Door")
	TSubclassOf<ADoorActor> DoorActorClass;

	/* 문을 방 가장자리 어디에 붙일지 계산할 때 쓰는 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Door")
	float RoomHalfExtent = 1000.f;

	UPROPERTY()
	TObjectPtr<URoomSizeSettings> CachedRoomSizeSettings;

	UPROPERTY()
	FVector InitialFloorMeshScale = FVector::OneVector;

	UPROPERTY()
	bool bHasInitialFloorMeshScale = false;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Visual Floor")
	bool bGenerateVisualFloorTiles = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Visual Floor", meta=(EditCondition="bGenerateVisualFloorTiles"))
	TObjectPtr<UStaticMesh> VisualFloorTileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Visual Floor", meta=(EditCondition="bGenerateVisualFloorTiles", ClampMin="1"))
	int32 VisualFloorTileSizeInCells = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Visual Floor", meta=(EditCondition="bGenerateVisualFloorTiles", ClampMin="0.01"))
	float VisualFloorTileScaleMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Visual Floor", meta=(EditCondition="bGenerateVisualFloorTiles"))
	float VisualFloorTileZOffset = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Visual Floor", meta=(EditCondition="bGenerateVisualFloorTiles"))
	bool bHideBaseFloorMeshWhenUsingVisualTiles = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Visual Floor", meta=(EditCondition="bGenerateVisualFloorTiles"))
	bool bRandomizeVisualFloorTileRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence")
	bool bGenerateEdgeFences = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences"))
	TObjectPtr<UStaticMesh> EdgeFenceMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences", ClampMin="1"))
	int32 EdgeFenceSegmentSizeInCells = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences", ClampMin="0.01"))
	float EdgeFenceScaleMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences"))
	float EdgeFenceZOffset = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences"))
	float EdgeFenceInset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences", ClampMin="0.0"))
	float EdgeFenceDoorGapWidth = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences"))
	bool bEnableEdgeFenceCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences"))
	TObjectPtr<UStaticMesh> DoorSideBlockMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences", ClampMin="0.01"))
	float DoorSideBlockScaleMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Edge Fence", meta=(EditCondition="bGenerateEdgeFences"))
	float DoorSideBlockZOffset = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Trigger", meta=(ClampMin="0.0"))
	float RoomEnterTriggerHeight = 300.f;

	/* 큰 구조물 메쉬 후보를 담는 데이터 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Interior")
	TObjectPtr<URoomInteriorPropSet> LargeStructurePropSet;

	/* K-Means 성향 기반으로 스폰할 장애물 블루프린트 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Obstacle")
	TSubclassOf<AAnalysisObstacle> ObstacleClass;

	/* 장애물 돌파 시 소환할 몬스터 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Obstacle")
	TSubclassOf<ACharacter> BypassMonsterClass;

	/* ObstacleBypass=1.0일 때 배치할 최대 장애물 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Obstacle", meta=(ClampMin="0", ClampMax="5"))
	int32 MaxObstaclesPerRoom = 3;
	
	UPROPERTY()
	TArray<TObjectPtr<ADoorActor>> SpawnedDoors;

	/* 현재 방에서 생성한 경로 액터 목록
	   - 방이 다시 초기화될 때 함께 정리 */
	UPROPERTY()
	TArray<TObjectPtr<ARoomPathActor>> SpawnedPathActors;

	/* 성향 기반으로 동적 스폰된 장애물 목록 */
	UPROPERTY()
	TArray<TObjectPtr<AAnalysisObstacle>> SpawnedObstacles;

	/* 이 방에서 동적으로 생성한 큰 구조물 메쉬 컴포넌트 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> SpawnedStructureMeshes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> SpawnedVisualFloorTileMeshes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> SpawnedEdgeFenceMeshes;

	UPROPERTY()
	FRoomData CachedRoomData;

	/* 내부 생성에 사용한 맵 시드 보관 */
	UPROPERTY()
	int32 CachedMapSeed = 0;

	/* MapManager에서 전달받은 K-Means 성향 데이터 */
	FPlayerTendencyModifier CachedTendency;

	/* 시작방 기준 거리로 분류된 구역 */
	ERoomZone CachedZone = ERoomZone::Mid;

	/* 마지막 내부 생성 결과를 캐싱 */
	UPROPERTY()
	FRoomInteriorLayout CachedInteriorLayout;

	/* 내부 경로 생성기 */
	UPROPERTY()
	TObjectPtr<URoomInteriorGenerator> InteriorGenerator;

private:
	UPROPERTY()
	bool bRoomActivated = false;
	UPROPERTY()
	bool bRoomCleared = false;
	UPROPERTY()
	bool bMonstersSpawned = false;

	UFUNCTION()
	void OnRoomEnterTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void UpdateRoomEnterTriggerExtent();
	float GetEffectiveRoomHalfExtent() const;
	float GetEffectiveDoorInset() const;
	float GetEffectiveEntranceInset() const;
	float GetEffectiveTriggerHalfExtent() const;
	float GetEffectiveInteriorCellSize() const;
	float GetEffectiveInteriorMargin() const;
	void ApplyFloorMeshScale();
	void CheckPlayerInsideRoom();
	void CheckRoomClear();
	bool ShouldLockPortalsForCombat() const;
	void HandleCombatRoomCleared();
	void SetSpawnedDoorsPortalEnabled(bool bEnabled);
	void SpawnRoomMonsters();
	void ApplyBaseFloorVisibility();

	void ClearSpawnedDoors();
	/* 이 방이 소유하는 경로 액터들을 정리 */
	void ClearSpawnedPathActors();
	/* 이 방이 소유하는 큰 구조물 메쉬 컴포넌트를 정리 */
	void ClearSpawnedStructureMeshes();
	void ClearSpawnedVisualFloorTiles();
	void ClearSpawnedEdgeFences();
	/* 성향 기반으로 동적 스폰된 장애물 정리 */
	void ClearSpawnedObstacles();
	/* ObstacleBypass 성향에 따라 장애물 동적 스폰 */
	void SpawnObstaclesByTendency(const FPlayerTendencyModifier& Tendency);
	void SpawnConnectedDoors();
	void SpawnDoor(EDoorDirection Direction);

	/* 내부 경로 데이터를 생성하고 실제 path actor를 배치 */
	void GenerateAndSpawnInterior();
	void SpawnVisualFloorTiles();
	void SpawnEdgeFences();
	void SpawnEdgeFenceLine(
		const FVector& EdgeStart,
		const FVector& EdgeDirection,
		float EdgeLength,
		float Yaw,
		bool bHasDoorGap);
	void SpawnDoorSideBlocks(
		const FVector& EdgeStart,
		const FVector& EdgeDirection,
		float EdgeLength,
		float Yaw);
	bool IsInDoorGap(float DistanceAlongEdge, float EdgeLength) const;
	/* 생성된 경로 데이터를 바탕으로 전용 path actor를 스폰 */
	void SpawnGuaranteedPaths(const FRoomInteriorLayout& Layout);
	/* 큰 구조물 점유 결과를 실제 메쉬 컴포넌트로 배치 */
	void SpawnLargeStructureMeshes(const FRoomInteriorLayout& Layout);
	/* 셀 기반 구조 결과를 디버그 박스로 시각화 */
	void DrawInteriorCellDebug(const FRoomInteriorLayout& Layout) const;
	
	FTransform GetDoorTransform(EDoorDirection Direction) const;
	FIntPoint GetNeighborGridPos(EDoorDirection Direction) const;
};
