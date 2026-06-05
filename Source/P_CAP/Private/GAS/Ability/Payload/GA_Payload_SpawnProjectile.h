// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/Payload/GA_PayloadBase.h"
#include "GAS/Actors/CAP_ProjectileBase.h"
#include "GA_Payload_SpawnProjectile.generated.h"

/**
 * 
 */
UCLASS()
class UGA_Payload_SpawnProjectile : public UGA_PayloadBase
{
	GENERATED_BODY()

public:
	UGA_Payload_SpawnProjectile();

protected:
	virtual void ExecutePayloadLogic(const FGameplayEventData& EventData) override;
	void SpawnProjectile(FVector SpawnLoc, FRotator SpawnRot, FVector TargetLoc, const FGameplayEventData& EventData);
	class USceneComponent* FindHomingTarget(const FVector& SearchOrigin);
	virtual float GetPayloadTargetingRadius() override {return ExplosionRadius;}

	bool GetSpawnLocationFromNotify(const FGameplayEventData& EventData, class AActor* OwnerActor, FVector& OutSpawnLoc) const;
	void CalculateTargetAndRotation(const FGameplayEventData& EventData, FVector& InOutSpawnLoc, FRotator& InOutSpawnRot, FVector& OutTargetLoc) const;
	
	UPROPERTY(EditAnywhere, Category="Projectile")
	TSubclassOf<class ACAP_ProjectileBase> ProjectileClass;

	// 소환할 투사체 개수
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category="Projectile")
	int32 NumOfProjectiles = 1;
	// 여러개 소환되는 경우, 퍼지는 각도 
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition = "NumOfProjectiles > 1"), Category="Projectile")
	float SpreadAngle = 30.f;
	
	// 투사체 타입
	UPROPERTY(EditDefaultsOnly, Category="InitData")
	EProjectileType ProjectileType = EProjectileType::Straight;

	// 투사체 관통 횟수
	UPROPERTY(EditDefaultsOnly,meta=(EditCondition="ProjectileType == EProjectileType::Straight", EditConditionHides), meta = (ClampMin = "1"), Category="InitData")
	int32 MaxHitCount = 1;
	// 투사체 속도
	UPROPERTY(EditDefaultsOnly,meta=(EditCondition="ProjectileType != EProjectileType::Arc", EditConditionHides), Category="InitData")
	float ProjectileSpeed = 1000.f;
	// 날라갈 수 있는 최대 사거리
	UPROPERTY(EditDefaultsOnly,meta=(EditCondition="ProjectileType == EProjectileType::Straight", EditConditionHides), Category="InitData")
	float MaxDistance = 1500.f;
	// 범위 데미지 크기
	UPROPERTY(EditDefaultsOnly,meta=(EditCondition="ProjectileType == EProjectileType::Arc || ProjectileType == EProjectileType::Falling", EditConditionHides), Category="InitData")
	float ExplosionRadius = 0.f;
	
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Arc", EditConditionHides), Category="InitData")
	float ArcTension = 0.5f;
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Falling", EditConditionHides), Category="InitData")
	float FallingSpawnHeight = 1000.f;
	// 유도 가능한 적 인지 가능 거리
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Homing", EditConditionHides), Category="InitData")
	float HomingSearchRadius = 1500.f;
};
