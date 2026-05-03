// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "CAP_AoESpawner.generated.h"

UENUM(BlueprintType)
enum class EAoESpawnMethod : uint8
{
	RandomFalling		UMETA(DisplayName = "RandomFalling (랜덤 위치(SpawnAreaRadius반경)에 메테오식 투사체)"),
	TargetedFalling		UMETA(DisplayName = "TargetedFalling (SpawnAreaRadius 반경 내 몬스터 머리위에서 투사체 떨어뜨림)"),
	Homing				UMETA(DisplayName = "Homing (SpawnAreaRadius 반경 내 몬스터에게 유도)"),
};

USTRUCT(BlueprintType)
struct FAoESpawnerSetupData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAoESpawnMethod SpawnMethod = EAoESpawnMethod::RandomFalling;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ACAP_ProjectileBase> ProjectileClass = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumProjectiles = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnAreaRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExplosionRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FallingHeight = 600.f;
	
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
	void SpawnRandomFalling();
	void SpawnTargetedFalling();
	void SpawnHoming();

	TArray<class USceneComponent*> FindEnemyRootComponent();
};
