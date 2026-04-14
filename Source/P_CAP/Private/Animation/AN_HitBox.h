// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_HitBox.generated.h"

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
class UAN_HitBox : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY(EditAnywhere, Category="1. General")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category="1. General")
	FName SocketName = "root";

	// 공격 회수
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
	
	UPROPERTY(EditAnywhere, Category="3. Transform Offset")
	FVector LocalOffset = FVector(0.f);

	UPROPERTY(EditAnywhere, Category="3. Transform Offset")
	FRotator LocalRotation = FRotator::ZeroRotator;
	
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
