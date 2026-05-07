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

	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	UBoxComponent* OuterZone; 

	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	UBoxComponent* InnerZone; 

	// [변경] 하드코딩 제거 → 에디터에서 장애물 크기 조절 가능
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
};