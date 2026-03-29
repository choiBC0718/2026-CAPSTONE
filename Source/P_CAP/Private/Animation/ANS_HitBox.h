// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_HitBox.generated.h"

UENUM(BlueprintType)
enum class EHitboxShape : uint8
{
	Box UMETA(DisplayName = "Box (네모)"),
	Sphere UMETA(DisplayName = "Sphere (원형)"),
	Capsule UMETA(DisplayName = "Capsule (기둥)")
};

/**
 * 
 */
UCLASS()
class UANS_HitBox : public UAnimNotifyState
{
	GENERATED_BODY()
	public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, Category="1. General")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category="1. General")
	FName SocketName = "root";

	UPROPERTY(EditAnywhere, Category="1. General")
	int32 EventFireCount = 1;
	
	UPROPERTY(EditAnywhere, Category="2. Shape & Size")
	EHitboxShape ShapeType = EHitboxShape::Box;

	UPROPERTY(EditAnywhere, Category="2. Shape & Size", meta=(EditCondition="ShapeType==EHitboxShape::Box", EditConditionHides))
	FVector BoxExtent = FVector(50.f, 50.f, 50.f);

	UPROPERTY(EditAnywhere, Category="2. Shape & Size", meta=(EditCondition="ShapeType==EHitboxShape::Sphere", EditConditionHides))
	float SphereRadius = 50.f;

	UPROPERTY(EditAnywhere, Category="2. Shape & Size", meta=(EditCondition="ShapeType==EHitboxShape::Capsule", EditConditionHides))
	float CapsuleRadius = 40.f;

	UPROPERTY(EditAnywhere, Category="2. Shape & Size", meta=(EditCondition="ShapeType==EHitboxShape::Capsule", EditConditionHides))
	float CapsuleHalfHeight = 80.f;

	// --- [3. 위치 및 회전 (오프셋)] ---
	UPROPERTY(EditAnywhere, Category="3. Transform Offset")
	FVector LocalOffset = FVector(100.f, 0.f, 0.f); // 앞으로 얼마나 밀어낼지

	UPROPERTY(EditAnywhere, Category="3. Transform Offset")
	FRotator LocalRotation = FRotator::ZeroRotator;

	// --- [4. 에디터 프리뷰 (디버그)] ---
	UPROPERTY(EditAnywhere, Category="4. Debug")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, Category="4. Debug")
	FColor DebugColor = FColor::Red;

private:
	// 중복 타격 방지용 맵 (캐릭터별 관리)
	TMap<USkeletalMeshComponent*, TSet<AActor*>> HitActorsMap;

	// 에디터에서 눈으로 볼 수 있게 그려주는 내부 함수
	void DrawDebugShape(UWorld* World, const FVector& Location, const FRotator& Rotation) const;
};
