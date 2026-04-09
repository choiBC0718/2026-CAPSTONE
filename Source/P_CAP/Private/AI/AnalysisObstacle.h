// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AnalysisObstacle.generated.h"

class UBoxComponent;

UCLASS()
class P_CAP_API AAnalysisObstacle : public AActor
{
	GENERATED_BODY()
	
public:	
	AAnalysisObstacle();

protected:
	virtual void BeginPlay() override;

	// 접근 및 회피 감지용 (바깥쪽 큰 박스)
	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	UBoxComponent* OuterZone; 

	// 돌파 감지용 (안쪽 작은 박스)
	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	UBoxComponent* InnerZone; 

	UFUNCTION()
	void OnOuterOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOuterOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnInnerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bHasPassedThrough; // 중심부(Inner) 통과 여부 기록
	bool bIsTracking;       // 현재 플레이어가 Outer 영역 안에 있는지 추적 상태
};