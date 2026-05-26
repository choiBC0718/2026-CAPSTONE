// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "CAP_OverlapDamageActorBase.generated.h"

USTRUCT(BlueprintType)
struct FOverlapDamageActorInitData
{
	GENERATED_BODY()

	FGameplayEffectSpecHandle DamageSpecHandle;
	float LifeSpan = 5.f;
	float DamageTickRate = 0.5f;
	float CollisionRadius = 50.f;
	FGameplayTag CueTag;
};

UCLASS()
class ACAP_OverlapDamageActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ACAP_OverlapDamageActorBase();

	virtual void InitSkillActor(const FOverlapDamageActorInitData& InitData);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComp;

	FOverlapDamageActorInitData SkillData;

	UPROPERTY()
	TSet<AActor*> OverlappingActors;

	FTimerHandle DamageTickTimerHandle;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void ProcessDamageTick();
};
