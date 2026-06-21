// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Actors/CAP_AoESpawner.h"
#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "ItemBehavior_SpawnProjectile.generated.h"

/**
 * 
 */
UCLASS()
class UItemBehavior_SpawnProjectile : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	// 효과를 발동시킬 조건의 트리거 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Item.Trigger"), Category="Trigger")
	FGameplayTag TriggerEventTag;
	// 발동위한 트리거 발동 횟수
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1"), Category="Trigger")
	int32 RequiredTriggerCount = 1;
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0", ClampMax="100.0"), Category="Trigger")
	float TriggerChance = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile Setting")
	FDamageCalculationData DamageCalculation;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile Setting")
	TSubclassOf<class ACAP_ProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category="Projectile Setting")
	EProjectileType ProjectileType = EProjectileType::Straight;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1"), Category="Projectile Setting")
	int32 NumProjectiles = 1;

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="NumProjectiles > 1", EditConditionHides), Category="Projectile Setting")
	float SpreadAngle = 0.f;

	// 캐릭터 기준으로 얼마나 떨어져서 스폰될 것인가? (X: 앞뒤, Y: 좌우, Z: 위아래)
	UPROPERTY(EditDefaultsOnly, Category="Projectile Setting")
	FVector SpawnOffset = FVector(50.f, 0.f, 50.f);

	// [공통 & 직진/호밍/폴링] 투사체 속도 (Arc는 자체 계산하므로 숨김)
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType != EProjectileType::Arc", EditConditionHides), Category="Projectile Setting")
	float ProjectileSpeed = 1000.f;

	// [직진 & 호밍] 최대 사거리
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Straight || ProjectileType == EProjectileType::Homing", EditConditionHides), Category="Projectile Setting")
	float MaxDistance = 1500.f;

	// [직진 & 호밍] 관통 횟수
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Straight || ProjectileType == EProjectileType::Homing", EditConditionHides, ClampMin="1"), Category="Projectile Setting")
	int32 MaxHitCount = 1;

	// [호밍 & Targeted Falling] 유도/타격 가능한 적 인지 거리
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Homing || ProjectileType == EProjectileType::Falling", EditConditionHides), Category="Projectile Setting")
	float SearchRadius = 1500.f;

	// [폴링 전용] 무작위 위치에 떨어질 것인가? (false면 적 머리 위 확정 타격)
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Falling", EditConditionHides), Category="Projectile Setting")
	bool bIsRandomFalling = false;

	// [폴링 전용] 스폰 높이
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Falling", EditConditionHides), Category="Projectile Setting")
	float FallingSpawnHeight = 1000.f;

	// [무작위 폴링 전용] 낙하 무작위 반경
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Falling && bIsRandomFalling", EditConditionHides), Category="Projectile Setting")
	float RandomFallingRadius = 500.f;

	// [폴링 전용] 폭발 반경
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="ProjectileType == EProjectileType::Falling", EditConditionHides), Category="Projectile Setting")
	float ExplosionRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category="Projectile Setting", meta=(Categories="GameplayCue.Hit"))
	FGameplayTag HitCueTag;
	
	virtual void OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const override;
	virtual void OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const override;

	virtual void OnEventReceived(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const override;
private:
	bool CheckTriggerCondition(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const;
	USceneComponent* FindNearestTarget(const FVector& Origin, class UWorld* World, AActor* IgnoredActor) const;
};
