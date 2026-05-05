// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GameFramework/Pawn.h"
#include "CAP_FlyingPetPawn.generated.h"

UCLASS()
class ACAP_FlyingPetPawn : public APawn
{
	GENERATED_BODY()

public:
	ACAP_FlyingPetPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	class USphereComponent* SphereComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	class USkeletalMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	class UFloatingPawnMovement* MovementComp;

	// 플레이어 캐싱
	UPROPERTY(BlueprintReadOnly)
	AActor* Player;

	void InitializePet(AActor* InPlayer);
	UFUNCTION()
	void ExecuteAttack(AActor* TargetActor);

	void SetVisualAndOffset(class USkeletalMesh* InMesh, FVector InOffset);
	void SetStats(ESkillDamageType InDamageType, float InBaseDamage, float InDamageMultiplier);
private:
	FGameplayTag BaseDamageTag;
	FGameplayTag DamageMultiplierTag;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	FVector FollowOffset = FVector(-100.f, 0.f, 150.f);
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float FollowSpeed = 1.5f;
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float TeleportDistance = 1500.f;

	ESkillDamageType PetDamageType;
	float PetBaseDamage;
	float PetDamageMultiplier;
};
