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
	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	UBoxComponent* GoalZone;

	// [변경됨] 기준 클리어 타임을 현실적인 시간(예: 120초)으로 대폭 낮춥니다.
	UPROPERTY(EditAnywhere, Category = "Normalization", meta=(ClampMin="10.0"))
	float MaxExpectedPlayTime = 120.0f;

	// [추가됨] 전체 맵 타일 중 몇 %를 밟았을 때 '탐험도 100점(1.0)'으로 쳐줄 것인지 결정합니다.
	// 0.4 = 맵의 40%만 돌아다녀도 탐험형 수치가 1.0으로 꽉 찹니다.
	UPROPERTY(EditAnywhere, Category = "Normalization", meta=(ClampMin="0.1", ClampMax="1.0"))
	float ExpectedMaxVisitRatio = 0.4f;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};