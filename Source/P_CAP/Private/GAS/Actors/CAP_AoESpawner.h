// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_ProjectileBase.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "CAP_AoESpawner.generated.h"

USTRUCT(BlueprintType)
struct FAoESpawnerSetupData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	EProjectileType ProjectileType = EProjectileType::Falling;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="ProjectileType == EProjectileType::Falling"), Category="Setting")
	bool bRandomLocationInArea = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	TSubclassOf<class ACAP_ProjectileBase> ProjectileClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	float DelayTime = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	int32 NumProjectiles = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="ProjectileType == EProjectileType::Straight || ProjectileType == EProjectileType::Homing"))
	float SpreadAngle = 0.f;

	// Homing또는 대상 추적이 필요할 땐 추적 가능 거리 // 랜덤 Falling인 경우 캐릭터로 부터 떨어진 거리 안으로 Falling 투사체 소환
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	float SpawnAreaRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	float ExplosionRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	float FallingHeight = 600.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	float ProjectileSpeed = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	float MaxDistance = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Setting")
	int32 MaxHitCount = 1;
	
	UPROPERTY()
	FGameplayTag TriggerItemProjHitTag = FGameplayTag::RequestGameplayTag("Item.Trigger.Hit.Item");

	FGameplayEffectSpecHandle DamageSpecHandle = nullptr;
	FGameplayTag CueTag;
};

UCLASS()
class ACAP_AoESpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ACAP_AoESpawner();

	void InitializeSpawner(const FAoESpawnerSetupData& InData);

protected:
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* Root;
	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* Decal;

private:
	FAoESpawnerSetupData SetupData;
	FTimerHandle SpawnTimerHandle;

	void OnDelayFinished();
	
	void SpawnOnEnemies();	// Homing, Targeted Falling
	void SpawnRandomly();	// Random Falling
	void SpawnStraight();	//Straight

	void SpawnProjectileDeferred(const FVector& SpawnLoc, const FVector& LaunchDir, USceneComponent* HomingTarget = nullptr);

	TArray<class USceneComponent*> FindEnemyRootComponent();
};
