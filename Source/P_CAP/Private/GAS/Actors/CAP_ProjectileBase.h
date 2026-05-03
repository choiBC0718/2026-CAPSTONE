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

UCLASS()
class ACAP_ProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:
	ACAP_ProjectileBase();

	virtual void BeginPlay() override;

	void InitProjectile(FVector TargetOrDirection, float InExplosionRadius, float ArcTension, FGameplayEffectSpecHandle InHitEffectSpecHandle, FGameplayTag CueTag, FGameplayTag InTriggerEventTag, USceneComponent* HomingTarget = nullptr);

	UPROPERTY(EditDefaultsOnly, Category="Setting")
	EProjectileType ProjectileType = EProjectileType::Straight;
	UPROPERTY()
	int32 MaxHitCount =1;
	
private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComp;
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjMovementComp;

	UPROPERTY(EditDefaultsOnly, Category="Setting")
	float ProjectileSpeed = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category="Setting")
	float MaxDistance = 1500.f;

	float ExplosionRadius = 0.f;

	FGameplayTag TriggerEventTag;
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
};
