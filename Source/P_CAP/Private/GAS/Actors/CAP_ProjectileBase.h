// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "CAP_ProjectileBase.generated.h"

UCLASS()
class ACAP_ProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ACAP_ProjectileBase();

	virtual void BeginPlay() override;

	void InitStraightProjectile(FVector Direction, FGameplayEffectSpecHandle InHitEffectSpecHandle, FGameplayTag CueTag);
	void InitArcProjectile(FVector TargetLoc, float ArcTension,float InExplosionRadius, FGameplayEffectSpecHandle InHitEffectSpecHandle, FGameplayTag CueTag);
	
private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComp;
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjMovementComp;

	UPROPERTY(EditDefaultsOnly, Category="Settings")
	float ProjectileSpeed = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category="Settings")
	float MaxDistance = 1500.f;

	float ExplosionRadius = 0.f;
	
	FGameplayTag HitGameplayCueTag;
	FGameplayEffectSpecHandle HitEffectHandle;
	FTimerHandle ProjTimerHandle;

	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void TravelMaxDistanceReached();
	void SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult);
};
