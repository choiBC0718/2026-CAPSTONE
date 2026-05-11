// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "CAP_ProjectileBase.generated.h"

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Straight,
	Arc,
	Falling,
	Homing,
};

USTRUCT(BlueprintType)
struct FProjectileInitData
{
	GENERATED_BODY()
	
	EProjectileType ProjectileType = EProjectileType::Straight;
	FVector LaunchDir = FVector::ZeroVector;
	float ProjectileSpeed = 1000.f;
	float MaxDistance = 1000.f;
	float ExplosionRadius = 0.f;
	float ArcTension = 0.5f;
	int32 MaxHitCount = 1;
	FGameplayEffectSpecHandle DamageSpecHandle;
	FGameplayTag CueTag;
	FGameplayTag HitTriggerTag;
	TWeakObjectPtr<USceneComponent> HomingTarget = nullptr;
	FVector TargetLocation;
};

UCLASS()
class ACAP_ProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:
	ACAP_ProjectileBase();

	virtual void BeginPlay() override;
	
	void InitProjectile(const FProjectileInitData& InitData);

	
private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComp;
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjMovementComp;
	UPROPERTY(VisibleAnywhere)
	class UParticleSystemComponent* TrailParticleComp;

	UPROPERTY(EditDefaultsOnly, Category="Setting")
	class UParticleSystem* HitVFX;
	
	EProjectileType ProjectileType = EProjectileType::Straight;
	float ProjectileSpeed = 1000.f;
	float MaxDistance = 1500.f;
	float ExplosionRadius = 0.f;
	int32 MaxHitCount =1;

	FGameplayTag HitTriggerTag;
	FGameplayTag HitGameplayCueTag;
	FGameplayEffectSpecHandle HitEffectHandle;
	FTimerHandle ProjTimerHandle;
	
	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void TravelMaxDistanceReached();
	void SendLocalGameplayCue( const FHitResult& HitResult);

	void ProcessStraightHit(AActor* OtherActor, const FHitResult& SweepResult);
	void ProcessExplosiveHit(const FHitResult& SweepResult);
	
	int32 CurrentHitCount =0;
	UPROPERTY()
	TSet<AActor*> HitActors;

	void StraightTypeInit();
};
