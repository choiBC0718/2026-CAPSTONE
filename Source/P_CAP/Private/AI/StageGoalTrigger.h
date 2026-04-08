// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "StageGoalTrigger.generated.h" 

UCLASS()
class P_CAP_API AStageGoalTrigger : public AActor 
{
	GENERATED_BODY()
	
public:	
	AStageGoalTrigger();

protected:
	// 플레이어가 밟을 투명한 결승선 박스
	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	UBoxComponent* GoalZone;

	// 박스에 캐릭터가 닿았을 때(Overlap) 실행될 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};