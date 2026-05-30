#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "AnalysisObstacle.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class P_CAP_API AAnalysisObstacle : public AActor
{
	GENERATED_BODY()

public:
	AAnalysisObstacle();

	// 돌파 시 소환할 몬스터 클래스 (RoomActor에서 스폰 후 SetBypassMonsterClass로 주입)
	UPROPERTY(EditAnywhere, Category = "Bypass")
	TSubclassOf<ACharacter> BypassMonsterClass;

	// 장애물 중심에서 몬스터를 소환할 반경
	UPROPERTY(EditAnywhere, Category = "Bypass")
	float BypassSpawnRadius = 500.f;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Visual")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	UBoxComponent* OuterZone;

	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	UBoxComponent* InnerZone;

	UPROPERTY(EditAnywhere, Category = "Trigger|Size")
	FVector OuterZoneExtent = FVector(300.f, 300.f, 200.f);

	UPROPERTY(EditAnywhere, Category = "Trigger|Size")
	FVector InnerZoneExtent = FVector(100.f, 100.f, 200.f);

	UFUNCTION()
	void OnOuterOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOuterOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnInnerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bHasPassedThrough;
	bool bIsTracking;
	bool bMonsterSpawned;  // 장애물당 한 번만 소환
};